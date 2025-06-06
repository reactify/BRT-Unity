
#pragma once

#define NOMINMAX
#include <cfloat>
#include "BRTLibrary.h"
#include "VersionedParameters.h"

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
    //==========================================================================
    struct Parameters
    {
        bool bypassed = false;
        bool spatializationEnabled = true;
        bool interpolationEnabled = true;
        bool itdSimulationEnabled = true;
        bool nearFieldEffectEnabled = true;
        bool parallaxCorrectionEnabled = true;
        bool distanceAttenuationEnabled = true;
    };
    
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
    bool createListenerModel (const char* listenerModelId);
    
    int addSoundSource();
    bool createSoundSource (const char* soundSourceId);
    bool removeSoundSource (const char* soundSourceId);

    // Update params safely from any other thread:
    template <typename Func>
    void updateParameters (Func&& f) { params.update (std::forward<Func> (f)); }
    
    BRTBase::CBRTManager brtManager;
    std::shared_ptr<BRTBase::CListener> listener;
    
private:
    //==========================================================================
    BRTLibraryWrapper (int sampleRate, int bufferSize);
    ~BRTLibraryWrapper();
    
    int getNextSoundSourceId();
    void releaseSoundSourceId (int soundSourceId);
    
    // Access params read-only from audio thread:
    const Parameters& getParameters() const noexcept    {  return params.get(); }
    void updateParameters (const Parameters& params);

    // Shared instance
    static std::atomic<BRTLibraryWrapper*> brtInstance;
    
    Common::CGlobalParameters globalParameters;
    
    std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> listenerModels;
    int sampleRate, bufferSize;
    CMonoBuffer<float> outLeftBuffer;
    CMonoBuffer<float> outRightBuffer;
    std::bitset<128> soundSourceIds;
    std::atomic<bool> suspended;
    std::mutex mutex;
    
    uint32_t lastParamVersion = 0;
    VersionedParameters<Parameters> params;
};


//==============================================================================
template <typename ListenerModelType>
inline bool BRTLibraryWrapper::createListenerModel (const char* listenerModelId)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    if (auto listenerModel = brtManager.CreateListenerModel<ListenerModelType> (listenerModelId))
    {
        listenerModels.emplace_back (std::move (listenerModel));
        return true;
    }
}

} // namespace BRTUnity
