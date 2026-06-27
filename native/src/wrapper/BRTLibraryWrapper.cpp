
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"

namespace BRTUnity
{

//==============================================================================
std::shared_ptr<BRTLibraryWrapper> BRTLibraryWrapper::brtInstance { nullptr };

std::shared_ptr<BRTLibraryWrapper> BRTLibraryWrapper::instance() noexcept
{
    return std::atomic_load_explicit (&brtInstance, std::memory_order_acquire);
}

//==============================================================================
BRTLibraryWrapper::ScopedSetup::ScopedSetup (BRTLibraryWrapper& w)
  : wrapper(w)
{
    wrapper.setupCounter.fetch_add (1, std::memory_order_release);
    wrapper.brtManager.BeginSetup();
}

BRTLibraryWrapper::ScopedSetup::~ScopedSetup()
{
    wrapper.brtManager.EndSetup();
    wrapper.setupCounter.fetch_sub (1, std::memory_order_release);
}

//==============================================================================
BRTLibraryWrapper::BRTLibraryWrapper (int sampleRate_, int bufferSize_)
  : sampleRate (sampleRate_), bufferSize (bufferSize_)
{
    globalParameters.SetSampleRate (sampleRate);
    globalParameters.SetBufferSize (bufferSize);

    outLeftBuffer.resize (bufferSize);
    outRightBuffer.resize (bufferSize);
    
    auto& errorHandler = Common::CErrorHandler::Instance();
    errorHandler.SetAssertMode (ASSERT_MODE_EMPTY);
    errorHandler.SetVerbosityMode (VERBOSITY_MODE_ALL);
    errorHandler.SetErrorLogFile ("/Users/ragnaringi/Desktop/BRT_Log.txt");
}

BRTLibraryWrapper::~BRTLibraryWrapper()
{
}

//==============================================================================
void BRTLibraryWrapper::initOrReplace (int sampleRate, int bufferSize)
{
    auto current = std::atomic_load_explicit(&brtInstance, std::memory_order_acquire);

    if (current && current->isCompatible (sampleRate, bufferSize))
        return;

    BRT_Log (0, "[BRTLibraryWrapper] Creating new instance");
    auto newInstance = std::shared_ptr<BRTLibraryWrapper>
    (
        new BRTLibraryWrapper (sampleRate, bufferSize),
        [] (BRTLibraryWrapper* p)
        {
            delete p;
        }
    );
    std::atomic_store_explicit (&brtInstance, newInstance, std::memory_order_release);
}

//==============================================================================
void BRTLibraryWrapper::destroy()
{
    BRT_Log (0, "[BRTLibraryWrapper] Destroying old instance");
    std::atomic_store_explicit (&brtInstance, std::shared_ptr<BRTLibraryWrapper>{}, std::memory_order_release);
}

//==============================================================================
bool BRTLibraryWrapper::isCompatible (int newSampleRate, int newBufferSize) const noexcept
{
    return sampleRate == newSampleRate && bufferSize == newBufferSize;
}

bool BRTLibraryWrapper::isSuspended() const noexcept
{
    return setupCounter.load (std::memory_order_acquire) > 0;
}

//==============================================================================
void BRTLibraryWrapper::process (float* inBuffer, float* outBuffer,
                                 unsigned int length, int inCh, int outCh) noexcept
{
    if (isSuspended())
    {
        std::fill (outBuffer, outBuffer + length * outCh, 0.0f);
        return;
    }
    
    auto snapshot = SpatializerRegistry::instance().get();

    for (const auto& [id, s] : *snapshot)
    {
        auto source = brtManager.GetSoundSource (std::to_string (id));
        if (! source)
            continue;

        source->SetSourceTransform (s.sourceTransform);
        source->SetBuffer (s.buffer);
    }

    brtManager.ProcessAll();

    auto localListener = getListener();
    if (localListener != nullptr)
    {
        localListener->GetBuffers (outLeftBuffer, outRightBuffer);
    }

    for (size_t i = 0; i < length; ++i)
    {
        outBuffer[i * 2 + 0] = inBuffer[i * 2 + 0] + outLeftBuffer[i];
        outBuffer[i * 2 + 1] = inBuffer[i * 2 + 1] + outRightBuffer[i];
    }
}

//==============================================================================
bool BRTLibraryWrapper::createListener (const char* listenerID)
{
    const ScopedSetup guard (*this);
    
    if (auto newListener = brtManager.CreateListener<BRTBase::CListener> (listenerID))
    {
        std::atomic_store_explicit (&listener, newListener, std::memory_order_release);
        return true;
    }
    
    return false;
}

bool BRTLibraryWrapper::removeListener (const char* listenerID)
{
    const ScopedSetup guard (*this);
    
    auto success = brtManager.RemoveListener (listenerID);
    
    if (brtManager.GetListenerIDs().size() == 0)
        std::atomic_store_explicit (&listener, std::shared_ptr<BRTBase::CListener>{}, std::memory_order_release);
    
    return success;
}

bool BRTLibraryWrapper::removeListenerModel (const char* listenerModelID)
{
    const ScopedSetup guard (*this);
    return brtManager.RemoveListenerModel (listenerModelID);
}

bool BRTLibraryWrapper::connectListenerModel (const char* listenerModelID, const char* listenerID)
{
    const ScopedSetup guard (*this);
    
    if (auto listener = brtManager.GetListener (listenerID))
    {
        if (! listener->ConnectListenerModel (listenerModelID))
        {
            BRT_Log (2, "BRT: Error connecting listener model");
            return false;
        }
        
        BRT_Log (0,"Connected listener model " + std::string (listenerModelID));
        return true;
    }
    
    BRT_Log (2, "BRT: No listener found");
    return false;
}

bool BRTLibraryWrapper::removeEnvironmentModel (const char* environmentModelID)
{
    const ScopedSetup guard (*this);
    return brtManager.RemoveEnvironmentModel (environmentModelID);
}

bool BRTLibraryWrapper::connectEnvironmentModel (const char* environmentModelID, const char* listenerModelID)
{
    const ScopedSetup guard (*this);
    
    if (auto listenerModel = brtManager.GetListenerModel<BRTListenerModel::CListenerModelBase> (listenerModelID))
    {
        if (! listenerModel->ConnectEnvironmentModel (environmentModelID))
        {
            BRT_Log (2, "BRT: Error connecting environment model");
            return false;
        }
        
        BRT_Log (0,"Connected environment model " + std::string (environmentModelID) + " to listener model " + std::string (listenerModelID));
        return true;
    }
    
    BRT_Log (2, "BRT: No listener model " + std::string (listenerModelID) + " found");
    return false;
}

void BRTLibraryWrapper::clearGraph()
{
    if (auto brt = instance())
    {
        auto localSampleRate = brt->sampleRate;
        auto localBufferSize = brt->bufferSize;
        
        destroy();
        initOrReplace (localSampleRate, localBufferSize);
    }
    
//    for (const auto& [id, s] : *snapshot)
//        if (auto source = brtManager.GetSoundSource (std::to_string (id)))
//            for (auto listenerModel : getListenerModels())
//                listenerModel->DisconnectSoundSource (std::to_string (id));
//    
//    for (auto environmentModelID : brtManager.GetEnvironmentModelIDs())
//        brtManager.RemoveEnvironmentModel (environmentModelID);
//    
//    for (auto listenerModelID : brtManager.GetListenerModelIDs())
//        brtManager.RemoveListenerModel (listenerModelID);
//    
//    for (auto listenerID : brtManager.GetListenerIDs())
//        brtManager.RemoveListener (listenerID);
}

bool BRTLibraryWrapper::createSoundSource (const char* soundSourceId, bool autoConnect)
{
    const ScopedSetup guard (*this);
    
    if (auto soundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceDirectivityModel> (soundSourceId))
    {
        if (autoConnect)
            autoConnectSoundSource (soundSourceId);
        
        return true;
    }
    
    return false;
}

bool BRTLibraryWrapper::removeSoundSource (const char* soundSourceId)
{
    const ScopedSetup guard (*this);
    
    for (const auto& listenerModel : getListenerModels())
        listenerModel->DisconnectSoundSource (soundSourceId);
    
    for (const auto& listenerModel : getEnvironmentModels())
        listenerModel->DisconnectSoundSource (soundSourceId);
    
    return brtManager.RemoveSoundSource (soundSourceId);
}

bool BRTLibraryWrapper::connectSoundSource (std::string soundSourceID, std::string modelID)
{
    const ScopedSetup guard (*this);
    
    BRT_Log (0, "Connecting sound source " + soundSourceID + " to model " + modelID);
    
    if (auto environmentModel = brtManager.GetEnvironmentModel<BRTEnvironmentModel::CEnviromentModelBase> (modelID))
        return environmentModel->ConnectSoundSource (soundSourceID);
    
    if (auto listenerModel = brtManager.GetListenerModel<BRTListenerModel::CListenerModelBase> (modelID))
        return listenerModel->ConnectSoundSource (soundSourceID);
    
    BRT_Log (2, "Error connecting sound source " + soundSourceID + " to model " + modelID);
    return false;
}

bool BRTLibraryWrapper::disconnectSoundSource (std::string soundSourceID, std::string modelID)
{
    const ScopedSetup guard (*this);
    
    if (auto environmentModel = brtManager.GetEnvironmentModel<BRTEnvironmentModel::CEnviromentModelBase> (modelID))
        return environmentModel->DisconnectSoundSource (soundSourceID);
    
    if (auto listenerModel = brtManager.GetListenerModel<BRTListenerModel::CListenerModelBase> (modelID))
        return listenerModel->DisconnectSoundSource (soundSourceID);
    
    BRT_Log (2, "Error disconnecting sound source " + soundSourceID + " to model " + modelID);
    return false;
}

bool BRTLibraryWrapper::setHRTF (const char* hrtfFile)
{
    auto hrtf = std::make_shared<BRTServices::CSphericalInterpolatedFIRTable>();
    
    if (! AppUtils::LoadHRTFSofaFile (hrtfFile, hrtf))
    {
        BRT_Log (2, "Error loading SOFA HRTF");
        return false;
    }
    
    const ScopedSetup guard (*this);

    auto localListener = getListener();
    if (! localListener)
        return false;
    
    return localListener->SetHRTF (hrtf);
}

bool BRTLibraryWrapper::setNFCFilter (const char* nfcFilterFile)
{
    auto sosFilter = std::make_shared<BRTServices::CSphericalSOSTable>();
    
    if (! AppUtils::LoadNearFieldSOSFilter (nfcFilterFile, sosFilter))
    {
        BRT_Log (2, "Error loading SOFA NFC file");
        return false;
    }
    
    const ScopedSetup guard (*this);

    auto localListener = getListener();
    if (! localListener)
        return false;
    
    return localListener->SetNearFieldCompensationFilters (sosFilter);
}

bool BRTLibraryWrapper::setBRIR (const char* brirFile)
{
    auto brir = std::make_shared<BRTServices::CSphericalFIRTable>();
    
    if (! AppUtils::LoadBRIRSofaFile (brirFile, brir, 0, 0, 0, 0))
    {
        BRT_Log (2, "Error loading SOFA BRIR file");
        return false;
    }
    
    const ScopedSetup guard (*this);
    
    auto localListener = getListener();
    if (! localListener)
        return false;
    
    return localListener->SetHRBRIR (brir);
}

bool BRTLibraryWrapper::setDirectivityTF (std::string soundSourceID, const char* directivityFile)
{
    auto soundSource = brtManager.GetSoundSource (soundSourceID);
    
    if (! soundSource)
    {
        BRT_Log (2, "Error setting DirectivityTF. Sound Source " + std::string (soundSourceID) + " not found");
        return false;
    }
    
    auto directivityTF = std::make_shared<BRTServices::CSphericalInterpolatedFIRTable>();
    
    if (! AppUtils::LoadDirectivityTFSofaFile (directivityFile, directivityTF))
    {
        BRT_Log (2, "Error loading SOFA DirectivityTF");
        return false;
    }
    
    return soundSource->SetDirectivity (directivityTF);
}

void BRTLibraryWrapper::setDirectivityEnabled (std::string soundSourceID, bool enabled)
{
    BRT_Log (0, "Setting DirectivityTF. Sound Source " + soundSourceID);
    
    if (auto soundSource = brtManager.GetSoundSource (soundSourceID))
    {
        soundSource->SetDirectivityEnable (enabled);
        
        int index = std::stoi (soundSourceID);
        
        auto snapshot = SpatializerRegistry::instance().get();
        SpatializerState state = snapshot.get()->at (index);
        state.enableDirectivity = enabled;
        SpatializerRegistry::instance().set (index, state);
        
        return;
    }
    
    BRT_Log (2, "Error setting DirectivityTF. Sound Source " + std::string (soundSourceID) + " not found");
}

auto setEnabled = [] (auto* obj, int8_t enabled, auto enableMethod, auto disableMethod) noexcept
{
    if (enabled > 0)
        (obj->*enableMethod)();
    else
        (obj->*disableMethod)();
};

void BRTLibraryWrapper::applyListenerModelParameters (const char* modelId,
                                                      const ListenerModelParameters* p)
{
    std::string log =
            "applyListenerModelParameters model=" + std::string (modelId) +
            " | enabled=" + std::to_string (p->enabled) +
            " gain=" + std::to_string (p->gain) +
            " spatial=" + std::to_string (p->spatializationEnabled) +
            " interp=" + std::to_string (p->interpolationEnabled) +
            " itd=" + std::to_string (p->itdSimulationEnabled) +
            " near=" + std::to_string (p->nearFieldEffectEnabled) +
            " parallax=" + std::to_string (p->parallaxCorrectionEnabled) +
            " distance=" + std::to_string (p->distanceAttenuationEnabled);

    BRT_Log (0, log);
    
    for (auto listenerModel : getListenerModels())
    {
        if (listenerModel->GetModelID() == modelId)
        {
            using namespace BRTListenerModel;
            
            setEnabled (listenerModel.get(), p->enabled,
                        &BRTBase::CModelBase::EnableModel,
                        &BRTBase::CModelBase::DisableModel);
            
            listenerModel->SetGain (p->gain);
            
            setEnabled (listenerModel.get(), p->spatializationEnabled,
                        &CListenerModelBase::EnableSpatialization,
                        &CListenerModelBase::DisableSpatialization);

            setEnabled (listenerModel.get(), p->interpolationEnabled,
                        &CListenerModelBase::EnableInterpolation,
                        &CListenerModelBase::DisableInterpolation);

            setEnabled (listenerModel.get(), p->itdSimulationEnabled,
                        &CListenerModelBase::EnableITDSimulation,
                        &CListenerModelBase::DisableITDSimulation);

            setEnabled (listenerModel.get(), p->nearFieldEffectEnabled,
                        &CListenerModelBase::EnableNearFieldEffect,
                        &CListenerModelBase::DisableNearFieldEffect);

            setEnabled (listenerModel.get(), p->parallaxCorrectionEnabled,
                        &CListenerModelBase::EnableParallaxCorrection,
                        &CListenerModelBase::DisableParallaxCorrection);

            setEnabled (listenerModel.get(), p->distanceAttenuationEnabled,
                        &CListenerModelBase::EnableDistanceAttenuation,
                        &CListenerModelBase::DisableDistanceAttenuation);
        }
    }
}

void BRTLibraryWrapper::applyEnvironmentModelParameters (const char* modelId,
                                                         const EnvironmentModelParameters* p)
{
    std::string log =
            "applyEnvironmentModelParameters model=" + std::string (modelId) +
            " | enabled=" + std::to_string (p->enabled) +
            " gain=" + std::to_string (p->gain) +
            " direct=" + std::to_string (p->directPathEnabled) +
            " reverb=" + std::to_string (p->reverbPathEnabled) +
            " delay=" + std::to_string (p->propagationDelayEnabled) +
            " distance=" + std::to_string (p->distanceAttenuationEnabled) +
            " distanceFactor=" + std::to_string (p->distanceAttenuationFactor) +
            " room=(" + std::to_string (p->roomLength) + "," +
                         std::to_string (p->roomWidth) + "," +
                         std::to_string (p->roomHeight) + ")";

    BRT_Log (0, log);
    
    for (auto environmentModel : getEnvironmentModels())
    {
        if (environmentModel->GetModelID() == modelId)
        {
            using namespace BRTEnvironmentModel;
            
            setEnabled (environmentModel.get(), p->enabled,
                        &BRTBase::CModelBase::EnableModel,
                        &BRTBase::CModelBase::DisableModel);
            
            environmentModel->SetGain (p->gain);
            
            setEnabled (environmentModel.get(), p->directPathEnabled,
                        &CEnviromentModelBase::EnableDirectPath,
                        &CEnviromentModelBase::DisableDirectPath);

            setEnabled (environmentModel.get(), p->reverbPathEnabled,
                        &CEnviromentModelBase::EnableReverbPath,
                        &CEnviromentModelBase::DisableReverbPath);

            setEnabled (environmentModel.get(), p->propagationDelayEnabled,
                        &CEnviromentModelBase::EnablePropagationDelay,
                        &CEnviromentModelBase::DisablePropagationDelay);
            
            setEnabled (environmentModel.get(), p->distanceAttenuationEnabled,
                        &CEnviromentModelBase::EnableDistanceAttenuation,
                        &CEnviromentModelBase::DisableDistanceAttenuation);
            
            environmentModel->SetDistanceAttenuationFactor (p->distanceAttenuationFactor);
            
            auto room = std::make_shared<BRTServices::CRoom>();
            room->SetupShoeBox (p->roomLength, p->roomWidth, p->roomHeight);

            for (int wallIndex = 0; wallIndex < ENVIRONMENT_MODEL_SHOEBOX_WALL_COUNT; ++wallIndex)
            {
                std::vector<float> absorptionBands;
                absorptionBands.reserve (ENVIRONMENT_MODEL_WALL_ABSORPTION_BAND_COUNT);

                std::string wallLog = "  wall " + std::to_string (wallIndex) + " bands=";

                for (int coeffIndex = 0; coeffIndex < ENVIRONMENT_MODEL_WALL_ABSORPTION_BAND_COUNT; ++coeffIndex)
                {
                    const int index = wallIndex * ENVIRONMENT_MODEL_WALL_ABSORPTION_BAND_COUNT + coeffIndex;

                    float v = std::clamp (p->wallAbsorptionCoefficients[index], 0.0f, 1.0f);
                    absorptionBands.push_back (v);

                    wallLog += std::to_string (v);

                    if (coeffIndex < ENVIRONMENT_MODEL_WALL_ABSORPTION_BAND_COUNT - 1)
                        wallLog += ",";
                }

                BRT_Log (0, wallLog);

                room->SetWallAbsortion (wallIndex, absorptionBands);
            }

            environmentModel->SetRoom (room);
        }
    }
}

std::shared_ptr<BRTBase::CListener> BRTLibraryWrapper::getListener() const noexcept
{
    return std::atomic_load_explicit (&listener, std::memory_order_acquire);
}

std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> BRTLibraryWrapper::getListenerModels()
{
    using ListenerModelBase = BRTListenerModel::CListenerModelBase;
    
    std::vector<std::shared_ptr<ListenerModelBase>> models;
    
    for (auto modelID : brtManager.GetListenerModelIDs())
        models.emplace_back (brtManager.GetListenerModel<ListenerModelBase> (modelID));
    
    return models;
}

std::vector<std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase>> BRTLibraryWrapper::getEnvironmentModels()
{
    using EnvironmentModelBase = BRTEnvironmentModel::CEnviromentModelBase;
    
    std::vector<std::shared_ptr<EnvironmentModelBase>> models;
    
    for (auto modelID : brtManager.GetEnvironmentModelIDs())
        models.emplace_back (brtManager.GetEnvironmentModel<EnvironmentModelBase> (modelID));
    
    return models;
}

} // namespace BRTUnity

