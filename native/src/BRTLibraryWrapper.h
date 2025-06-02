
#pragma once

#define NOMINMAX
#include <cfloat>
#include "BRTLibrary.h"

namespace BRTUnity
{

struct ScopedManagerSetup
{
    ScopedManagerSetup (BRTBase::CBRTManager& m)
      : manager (m)       { manager.BeginSetup(); }
    ~ScopedManagerSetup() { manager.EndSetup(); }
    BRTBase::CBRTManager& manager;
};

class BRTLibraryWrapper
{
public:
    // Audio-thread safe accessor
    static BRTLibraryWrapper* instance() noexcept;

    // Main-thread init/replacement
    static void initOrReplace (int sampleRate, int bufferSize);
    static void destroy();

    bool isCompatible (int sampleRate, int bufferSize) const noexcept;

    void process (float* in, float* out, unsigned int len, int inCh, int outCh) noexcept;

    int getNextSoundSourceId();
    
    void releaseSoundSourceId (int soundSourceId);
    
    BRTBase::CBRTManager brtManager;
    std::shared_ptr<BRTBase::CListener> listener;
    std::vector<std::shared_ptr<BRTServices::CHRTF>> hrtfs;
    std::vector<std::shared_ptr<BRTServices::CSOSFilters>> ilds;
    std::vector<std::shared_ptr<BRTServices::CHRBRIR>> brirs;
    
private:
    BRTLibraryWrapper (int sampleRate, int bufferSize);
    ~BRTLibraryWrapper();

    int sampleRate;
    int bufferSize;

    // Shared instance
    static std::atomic<BRTLibraryWrapper*> brtInstance;
    
    Common::CGlobalParameters globalParameters;
    // BRTBase::CBRTManager brtManager;
    // std::shared_ptr<BRTBase::CListener> listener;
    // std::vector<std::shared_ptr<BRTServices::CHRTF>> hrtfs;
    // std::vector<std::shared_ptr<BRTServices::CSOSFilters>> ilds;
    // std::vector<std::shared_ptr<BRTServices::CHRBRIR>> brirs;
    std::bitset<128> soundSourceIds;
};

} // namespace BRTUnity
