
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"
#include "Logging.h"

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

void BRTLibraryWrapper::clearGraph()
{
    const ScopedSetup guard (*this);
    
    auto snapshot = SpatializerRegistry::instance().get();

    for (const auto& [id, s] : *snapshot)
        if (auto source = brtManager.GetSoundSource (std::to_string (id)))
            for (auto listenerModel : getListenerModels())
                listenerModel->DisconnectSoundSource (std::to_string (id));
    
    for (auto listenerModelID : brtManager.GetListenerModelIDs())
        brtManager.RemoveListenerModel (listenerModelID);
    
    for (auto listenerID : brtManager.GetListenerIDs())
        brtManager.RemoveListener (listenerID);
    
    listener = nullptr;
}

bool BRTLibraryWrapper::createSoundSource (const char* soundSourceId, bool autoConnect)
{
    const ScopedSetup guard (*this);
    
    if (auto soundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceDirectivityModel> (soundSourceId))
    {
        if (autoConnect)
            for (auto listenerModel : getListenerModels())
                listenerModel->ConnectSoundSource (soundSourceId);
        
        return true;
    }
    
    return false;
}

bool BRTLibraryWrapper::removeSoundSource (const char* soundSourceId)
{
    const ScopedSetup guard (*this);
    
    for (const auto& listenerModel : getListenerModels())
        listenerModel->DisconnectSoundSource (soundSourceId);
    
    return brtManager.RemoveSoundSource (soundSourceId);
}

bool BRTLibraryWrapper::connectSoundSource (const char *soundSourceID, const char *listenerModelID)
{
    const ScopedSetup guard (*this);
    
    if (auto listenerModel = brtManager.GetListenerModel<BRTListenerModel::CListenerModelBase> (listenerModelID))
        return listenerModel->ConnectSoundSource (soundSourceID);
    
    BRT_Log (2, "Error connecting sound source " + std::string (soundSourceID) + " to listener model " + std::string (listenerModelID));
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

bool BRTLibraryWrapper::setDirectivityTF (const char* soundSourceID, const char* directivityFile)
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

void BRTLibraryWrapper::setDirectivityEnabled (const char* soundSourceID, bool enabled)
{
    BRT_Log (0, "Setting DirectivityTF. Sound Source " + std::string (soundSourceID));
    
    if (auto soundSource = brtManager.GetSoundSource (soundSourceID))
    {
        soundSource->SetDirectivityEnable (enabled);
        return;
    }
    
    BRT_Log (2, "Error setting DirectivityTF. Sound Source " + std::string (soundSourceID) + " not found");
}

void BRTLibraryWrapper::applyListenerModelParameters (const char* modelId,
                                                      const ListenerModelParameters* p)
{
    BRT_Log (0, "applyListenerModelParameters for model " + std::string (modelId));
    
    auto setEnabled = [] (auto* obj, bool enabled, auto enableMethod, auto disableMethod) noexcept
    {
        if (enabled)
            (obj->*enableMethod)();
        else
            (obj->*disableMethod)();
    };
    
    for (auto listenerModel : getListenerModels())
    {
        if (listenerModel->GetModelID() == modelId)
        {
            using namespace BRTListenerModel;
            
            setEnabled (listenerModel.get(), p->enabled,
                        &BRTBase::CModelBase::EnableModel,
                        &BRTBase::CModelBase::DisableModel);
            
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

} // namespace BRTUnity

