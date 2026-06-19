
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"
#include "Logging.h"

namespace BRTUnity
{

ScopedManagerSetup::ScopedManagerSetup (BRTBase::CBRTManager& m)
: manager (m)                             { manager.BeginSetup(); }
ScopedManagerSetup::~ScopedManagerSetup() { manager.EndSetup(); }

ScopedSuspendProcessing::ScopedSuspendProcessing (BRTLibraryWrapper& w)
: wrapper (w)
{ wrapper.suspendProcessing (true); }
ScopedSuspendProcessing::~ScopedSuspendProcessing()
{ wrapper.suspendProcessing (false); }

//==============================================================================
std::shared_ptr<BRTLibraryWrapper> BRTLibraryWrapper::brtInstance { nullptr };

//==============================================================================
std::shared_ptr<BRTLibraryWrapper> BRTLibraryWrapper::instance() noexcept
{
    return std::atomic_load_explicit (&brtInstance, std::memory_order_acquire);
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

//==============================================================================
void BRTLibraryWrapper::suspendProcessing (bool shouldBeSuspended) noexcept
{
    suspended.store (shouldBeSuspended, std::memory_order_release);
}

bool BRTLibraryWrapper::isSuspended() const noexcept
{
    return suspended.load (std::memory_order_acquire);
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

    brtManager.ProcessAll();

    if (listener != nullptr)
    {
        BRT_Log(0, "Process listener");
        listener->GetBuffers (outLeftBuffer, outRightBuffer);
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
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    if (auto listener = brtManager.CreateListener<BRTBase::CListener> (listenerID))
    {
        this->listener = listener;
        return true;
    }
    
    return false;
}

bool BRTLibraryWrapper::removeListener (const char* listenerID)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    auto success = brtManager.RemoveListener (listenerID);
    
    if (brtManager.GetListenerIDs().size() == 0)
        this->listener = nullptr;
    
    return success;
}

bool BRTLibraryWrapper::removeListenerModel (const char* listenerModelID)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    return brtManager.RemoveListenerModel (listenerModelID);
}

bool BRTLibraryWrapper::connectListenerModel (const char* listenerModelID, const char* listenerID)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup sm (brtManager);
    
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
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup sm (brtManager);
    
    for (auto listenerModelID : brtManager.GetListenerModelIDs())
        brtManager.RemoveListenerModel (listenerModelID);
    
    for (auto listenerID : brtManager.GetListenerIDs())
        brtManager.RemoveListener (listenerID);
    
    listener = nullptr;
}

bool BRTLibraryWrapper::createSoundSource (const char* soundSourceId, bool autoConnect)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
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
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    for (const auto& listenerModel : getListenerModels())
        listenerModel->DisconnectSoundSource (soundSourceId);
    
    return brtManager.RemoveSoundSource (soundSourceId);
}

bool BRTLibraryWrapper::setHRTF (const char* hrtfFile)
{
    if (! listener)
    {
        BRT_Log (2, "Error setting HRTF. No listener found");
        return false;
    }
    
    auto hrtf = std::make_shared<BRTServices::CSphericalInterpolatedFIRTable>();
    
    if (! AppUtils::LoadHRTFSofaFile (hrtfFile, hrtf))
    {
        BRT_Log (2, "Error loading SOFA HRTF");
        return false;
    }
    
    return listener->SetHRTF (hrtf);
}

bool BRTLibraryWrapper::setNFCFilter (const char* nfcFilterFile)
{
    if (! listener)
    {
        BRT_Log (2, "Error setting NFC. No listener found");
        return false;
    }
    
    auto sosFilter = std::make_shared<BRTServices::CSphericalSOSTable>();
    
    if (! AppUtils::LoadNearFieldSOSFilter (nfcFilterFile, sosFilter))
    {
        BRT_Log (2, "Error loading SOFA NFC file");
        return false;
    }
    
    return listener->SetNearFieldCompensationFilters (sosFilter);
}

bool BRTLibraryWrapper::setBRIR (const char* brirFile)
{
    BRT_Log (0, "[BRTLIbraryWrapper] Setting BRIR");
    
    if (! listener)
    {
        BRT_Log (2, "Error setting BRIR. No listener found");
        return false;
    }
    
    auto brir = std::make_shared<BRTServices::CSphericalFIRTable>();
    
    if (! AppUtils::LoadBRIRSofaFile (brirFile, brir, 0, 0, 0, 0))
    {
        BRT_Log (2, "Error loading SOFA BRIR file");
        return false;
    }
    
    BRT_Log (0, "[BRTLIbraryWrapper] Setting BRIR on listener");
    return listener->SetHRBRIR (brir);
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

std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> BRTLibraryWrapper::getListenerModels()
{
    using ListenerModelBase = BRTListenerModel::CListenerModelBase;
    
    std::vector<std::shared_ptr<ListenerModelBase>> models;
    
    for (auto modelID : brtManager.GetListenerModelIDs())
        models.emplace_back (brtManager.GetListenerModel<ListenerModelBase> (modelID));
    
    return models;
}

} // namespace BRTUnity

