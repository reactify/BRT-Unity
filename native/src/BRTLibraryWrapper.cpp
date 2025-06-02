
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"

namespace BRTUnity
{

inline void WriteLog (std::string logText)
{
    std::cerr << logText << std::endl;
}

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
}

BRTLibraryWrapper::~BRTLibraryWrapper()
{
    destroy();
}

void BRTLibraryWrapper::initOrReplace (int sampleRate, int bufferSize)
{
    BRTLibraryWrapper* existing = brtInstance.load (std::memory_order_acquire);

    if (existing && existing->isCompatible (sampleRate, bufferSize))
        return;

    auto* newInstance = new BRTLibraryWrapper (sampleRate, bufferSize);

    // Atomically replace the instance
    if (auto* old = brtInstance.exchange (newInstance, std::memory_order_acq_rel))
        delete old;
}

void BRTLibraryWrapper::destroy()
{
    if (auto* old = brtInstance.exchange (nullptr, std::memory_order_acq_rel))
        delete old;
}

bool BRTLibraryWrapper::isCompatible (int newSampleRate, int newBufferSize) const noexcept
{
    return sampleRate == newSampleRate && bufferSize == newBufferSize;
}

void BRTLibraryWrapper::process (float* in, float* out, unsigned int len, int inCh, int outCh) noexcept
{
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
