
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"

namespace BRTUnity
{

inline void WriteLog (std::string logText)
{
    std::cerr << logText << std::endl;
}

// Scoped guard class to automatically turn processing on/off
class ScopedSuspendProcessing
{
public:
    explicit ScopedSuspendProcessing (BRTLibraryWrapper& w)
      : wrapper (w)             { wrapper.suspendProcessing (true); }
    ~ScopedSuspendProcessing()  { wrapper.suspendProcessing (false); }
    ScopedSuspendProcessing (const ScopedSuspendProcessing&) = delete;
    ScopedSuspendProcessing& operator=(const ScopedSuspendProcessing&) = delete;
private:
    BRTLibraryWrapper& wrapper;
};

//==============================================================================
std::atomic<BRTLibraryWrapper*> BRTLibraryWrapper::brtInstance = nullptr;

BRTLibraryWrapper* BRTLibraryWrapper::instance() noexcept
{
    return brtInstance.load (std::memory_order_acquire);
}

BRTLibraryWrapper::BRTLibraryWrapper (int sampleRate_, int bufferSize_)
  : sampleRate (sampleRate_), bufferSize (bufferSize_)
{
    WriteLog ("BRT: BRTLibraryWrapper created for sampleRate "
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
    
    brtManager.ProcessAll();
    
    if (listener)
        listener->GetBuffers (outLeftBuffer, outRightBuffer);
    else
        WriteLog ("BRT: No listener found!!!");

    for (size_t i = 0; i < length; ++i)
    {
        outBuffer[i * 2 + 0] = outLeftBuffer[i];
        outBuffer[i * 2 + 1] = outRightBuffer[i];
    }
}

int BRTLibraryWrapper::getNextSoundSourceId()
{
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
    soundSourceIds.reset (soundSourceId);
}

}
