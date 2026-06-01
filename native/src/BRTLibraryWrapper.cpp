
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"

namespace BRTUnity
{

inline void WriteLog (std::string logText)
{
    std::cerr << logText << std::endl;
}

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

BRTLibraryWrapper* BRTLibraryWrapper::instance() noexcept
{
    return brtInstance.load (std::memory_order_acquire);
}

BRTLibraryWrapper::BRTLibraryWrapper (int sampleRate_, int bufferSize_)
  : sampleRate (sampleRate_), bufferSize (bufferSize_)
{
    WriteLog ("BRTLibraryWrapper created for sampleRate "
              + std::to_string (sampleRate)
              + " & bufferSize "
              + std::to_string (bufferSize));
    
    globalParameters.SetSampleRate (sampleRate);
    globalParameters.SetBufferSize (bufferSize);
    
    outLeftBuffer.resize (bufferSize);
    outRightBuffer.resize (bufferSize);
}

BRTLibraryWrapper::~BRTLibraryWrapper()
{
    destroy();
}

void BRTLibraryWrapper::initOrReplace (int sampleRate, int bufferSize)
{
    if (auto* oldInstance = brtInstance.load (std::memory_order_acquire))
    {
        if (! oldInstance->isCompatible (sampleRate, bufferSize))
        {
            const ScopedSuspendProcessing guard (*oldInstance);
            
            auto* newInstance = new BRTLibraryWrapper (sampleRate, bufferSize);
            brtInstance.store (newInstance, std::memory_order_release);
            
            delete oldInstance;
        }
    }
    else
    {
        auto* newInstance = new BRTLibraryWrapper (sampleRate, bufferSize);
        brtInstance.store (newInstance, std::memory_order_release);
    }
}

void BRTLibraryWrapper::destroy()
{
    if (auto* oldInstance = brtInstance.exchange (nullptr, std::memory_order_acq_rel))
    {
        const ScopedSuspendProcessing guard (*oldInstance);
        delete oldInstance;
    }
}

bool BRTLibraryWrapper::isCompatible (int newSampleRate, int newBufferSize) const noexcept
{
    return sampleRate == newSampleRate && bufferSize == newBufferSize;
}

void BRTLibraryWrapper::suspendProcessing (bool shouldBeSuspended) noexcept
{
    suspended.store (shouldBeSuspended, std::memory_order_release);
}

bool BRTLibraryWrapper::isSuspended() const noexcept
{
    return suspended.load (std::memory_order_acquire);
}

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
    else
        WriteLog ("No listener found!!!");
    
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
    
    if (auto soundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceSimpleModel> (soundSourceId))
        return true;
    
    return false;
}

bool BRTLibraryWrapper::removeSoundSource (const char* soundSourceId)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    releaseSoundSourceId (std::stoi (soundSourceId));
    
    for (const auto& listenerModel : listenerModels)
        listenerModel->DisconnectSoundSource (soundSourceId);
    
    return brtManager.RemoveSoundSource (soundSourceId);
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

} // namespace BRTUnity
