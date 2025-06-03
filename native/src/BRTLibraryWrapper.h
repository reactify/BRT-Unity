
#pragma once

#define NOMINMAX
#include <cfloat>
#include "BRTLibrary.h"

namespace BRTUnity
{

class BRTLibraryWrapper;

class ScopedManagerSetup
{
public:
    explicit ScopedManagerSetup (BRTBase::CBRTManager& m);
    ~ScopedManagerSetup();
    ScopedManagerSetup (const ScopedManagerSetup&) = delete;
    ScopedManagerSetup& operator=(const ScopedManagerSetup&) = delete;
private:
    BRTBase::CBRTManager& manager;
};

// Scoped guard class to automatically turn processing on/off
class ScopedSuspendProcessing
{
public:
    explicit ScopedSuspendProcessing (BRTLibraryWrapper& w);
    ~ScopedSuspendProcessing();
    ScopedSuspendProcessing (const ScopedSuspendProcessing&) = delete;
    ScopedSuspendProcessing& operator=(const ScopedSuspendProcessing&) = delete;
private:
    BRTLibraryWrapper& wrapper;
};

//==============================================================================
class BRTLibraryWrapper
{
public:
    // Audio-thread safe accessor
    static BRTLibraryWrapper* instance() noexcept;

    // Main-thread init/replacement
    static void initOrReplace (int sampleRate, int bufferSize);
    static void destroy();

    bool isCompatible (int sampleRate, int bufferSize) const noexcept;
    
    void suspendProcessing (bool shouldBeSuspended) noexcept;
    
    bool isSuspended() const noexcept;

    void process (float* in, float* out, unsigned int len, int inCh, int outCh) noexcept;
    
    template <typename ListenerModelType>
    bool createListenerModel (const char* listenerModelId)
    {
        const ScopedSuspendProcessing guard (*this);
        const ScopedManagerSetup managerSetup (brtManager);
        
        if (auto listenerModel = brtManager.CreateListenerModel<ListenerModelType> (listenerModelId))
        {
            listenerModels.emplace_back (std::move (listenerModel));
            return true;
        }
    }
    
    int addSoundSource();
    bool createSoundSource (const char* soundSourceId);
    bool removeSoundSource (const char* soundSourceId);
    
    BRTBase::CBRTManager brtManager;
    std::shared_ptr<BRTBase::CListener> listener;
    std::vector<std::shared_ptr<BRTServices::CHRTF>> hrtfs;
    std::vector<std::shared_ptr<BRTServices::CSOSFilters>> ilds;
    std::vector<std::shared_ptr<BRTServices::CHRBRIR>> brirs;
    
private:
    BRTLibraryWrapper (int sampleRate, int bufferSize);
    ~BRTLibraryWrapper();
    
    int getNextSoundSourceId();
    void releaseSoundSourceId (int soundSourceId);

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
    std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> listenerModels;
    CMonoBuffer<float> outLeftBuffer;
    CMonoBuffer<float> outRightBuffer;
    std::bitset<128> soundSourceIds;
    std::atomic<bool> suspended;
    std::mutex mutex;
};

} // namespace BRTUnity
