
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
std::atomic<BRTLibraryWrapper*> BRTLibraryWrapper::brtInstance = nullptr;
std::mutex BRTLibraryWrapper::retireMutex;
std::vector<BRTLibraryWrapper::RetiredItem> BRTLibraryWrapper::retired;

//==============================================================================
BRTLibraryWrapper* BRTLibraryWrapper::instance() noexcept
{
    return brtInstance.load (std::memory_order_acquire);
}

//==============================================================================
BRTLibraryWrapper::BRTLibraryWrapper (int sampleRate_, int bufferSize_)
  : sampleRate (sampleRate_), bufferSize (bufferSize_)
{
    globalParameters.SetSampleRate (sampleRate);
    globalParameters.SetBufferSize (bufferSize);

    outLeftBuffer.resize (bufferSize);
    outRightBuffer.resize (bufferSize);
}

BRTLibraryWrapper::~BRTLibraryWrapper()
{
}

//==============================================================================
void BRTLibraryWrapper::initOrReplace (int sampleRate, int bufferSize)
{
    auto* current = brtInstance.load (std::memory_order_acquire);

    if (current && current->isCompatible (sampleRate, bufferSize))
        return;

    BRT_Log (0, "[BRTLibraryWrapper] Creating new instance");
    
    auto* newInstance = new BRTLibraryWrapper (sampleRate, bufferSize);

    auto* old = brtInstance.exchange (newInstance, std::memory_order_acq_rel);

    if (old)
    {
        old->suspendProcessing (true);

        std::lock_guard<std::mutex> lock (retireMutex);
        retired.push_back({ old, 2 });
    }
}

//==============================================================================
void BRTLibraryWrapper::destroy()
{
    BRT_Log (0, "[BRTLibraryWrapper] Destroying old instance");
    
    auto* old = brtInstance.exchange (nullptr, std::memory_order_acq_rel);

    if (old)
    {
        old->suspendProcessing (true);

        std::lock_guard<std::mutex> lock (retireMutex);
        retired.push_back ({ old, 2 });
    }
}

//==============================================================================
void BRTLibraryWrapper::cleanup()
{
    std::lock_guard<std::mutex> lock (retireMutex);

    for (auto it = retired.begin(); it != retired.end(); )
    {
        if (--it->framesLeft <= 0)
        {
            delete it->ptr;
            it = retired.erase (it);
        }
        else
        {
            ++it;
        }
    }
}

//==============================================================================
bool BRTLibraryWrapper::isCompatible (int newSampleRate, int newBufferSize) const noexcept
{
    return sampleRate == newSampleRate && bufferSize == newBufferSize;
}

//==============================================================================
void BRTLibraryWrapper::suspendProcessing (bool shouldBeSuspended) noexcept
{
    suspended.store(shouldBeSuspended, std::memory_order_release);
}

bool BRTLibraryWrapper::isSuspended() const noexcept
{
    return suspended.load(std::memory_order_acquire);
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

    if (params.hasChanged (lastParamVersion))
    {
        const Parameters& p = params.get();

        if (p.bypassed)
            return;

        updateParameters (p);
        lastParamVersion = params.getVersion();
    }

    brtManager.ProcessAll();

    if (listener)
        listener->GetBuffers (outLeftBuffer, outRightBuffer);

    for (size_t i = 0; i < length; ++i)
    {
        outBuffer[i * 2 + 0] = inBuffer[i * 2 + 0] + outLeftBuffer[i];
        outBuffer[i * 2 + 1] = inBuffer[i * 2 + 1] + outRightBuffer[i];
    }
}

int BRTLibraryWrapper::addSoundSource()
{
    auto sourceId = getNextSoundSourceId();
    
    if (createSoundSource (std::to_string (sourceId).c_str()))
        return sourceId;
    
    releaseSoundSourceId (sourceId);
    return -1;
}

bool BRTLibraryWrapper::createSoundSource (const char* soundSourceId)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    if (auto soundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceOmnidirectionalModel> (soundSourceId))
    {
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
    
    releaseSoundSourceId (std::stoi (soundSourceId));
    
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
    
    BRT_Log (0, "[BRTLIbraryWrapper] Creating CSphericalFIRTable");
    auto brir = std::make_shared<BRTServices::CSphericalFIRTable>();
    
    BRT_Log (0, "[BRTLIbraryWrapper] Loading SOFA file");
    if (! AppUtils::LoadBRIRSofaFile (brirFile, brir, 0, 0, 0, 0))
    {
        BRT_Log (2, "Error loading SOFA BRIR file");
        return false;
    }
    
//    for (auto& _listenerModel : listenerModelsConnected) {
//        if (_listenerModel->GetListenerModelCharacteristics().SupportBRIR()) {    
    
    BRT_Log (0, "[BRTLIbraryWrapper] Setting BRIR on listener");
    return listener->SetHRBRIR (brir);
}

int BRTLibraryWrapper::getNextSoundSourceId()
{
    std::lock_guard<std::mutex> lock (mutex);
    
    for (int i = 0; i < soundSourceIds.size(); ++i)
    {
        if (! soundSourceIds.test (i))
        {
            soundSourceIds.set (i);
            return i;
        }
    }
    return -1; // out of IDs
}

void BRTLibraryWrapper::releaseSoundSourceId (int soundSourceId)
{
    if (soundSourceId < 0 || soundSourceId >= static_cast<int> (soundSourceIds.size()))
        return;
    
    std::lock_guard<std::mutex> lock (mutex);
    soundSourceIds.reset (soundSourceId);
}

void BRTLibraryWrapper::updateParameters (const Parameters& params)
{
    if (! listener)
        return;
    
    auto setEnabled = [] (auto* obj, bool enabled, auto enableMethod, auto disableMethod) noexcept
    {
        if (enabled)
            (obj->*enableMethod)();
        else
            (obj->*disableMethod)();
    };
    
    using namespace BRTBase;
    
    setEnabled (listener.get(), params.spatializationEnabled,
                &CListener::EnableSpatialization,
                &CListener::DisableSpatialization);
    
    setEnabled (listener.get(), params.interpolationEnabled,
                &CListener::EnableInterpolation,
                &CListener::DisableInterpolation);
    
    setEnabled (listener.get(), params.itdSimulationEnabled,
                &CListener::EnableITDSimulation,
                &CListener::DisableITDSimulation);
    
    setEnabled (listener.get(), params.nearFieldEffectEnabled,
                &CListener::EnableNearFieldEffect,
                &CListener::DisableNearFieldEffect);
    
    setEnabled (listener.get(), params.parallaxCorrectionEnabled,
                &CListener::EnableParallaxCorrection,
                &CListener::DisableParallaxCorrection);
    
    setEnabled (listener.get(), params.distanceAttenuationEnabled,
                &CListener::EnableDistanceAttenuation,
                &CListener::DisableDistanceAttenuation);
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
