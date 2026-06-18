
#pragma once

#define NOMINMAX
#include <algorithm>
#include <cfloat>
#include "BRTLibrary.h"
#include "Parameters.h"
#include "VersionedParameters.h"
#include "IdPool.h"

namespace BRTUnity
{

class GlobalIdPool {
public:
    static IdPool& instance() {
        static IdPool pool;   // thread-safe in C++11+
        return pool;
    }

private:
    GlobalIdPool() = default;
};


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
    static void cleanup();

    bool isCompatible (int sampleRate, int bufferSize) const noexcept;
    
    void suspendProcessing (bool shouldBeSuspended) noexcept;
    
    bool isSuspended() const noexcept;

    void process (float* in, float* out, unsigned int len, int inCh, int outCh) noexcept;
    
    //==========================================================================
    bool createListener (const char* listenerID);
    bool removeListener (const char* listenerID);
    
    template <typename ListenerModelType>
    bool createListenerModel (const char* listenerModelId);
    
    //==========================================================================
    int addSoundSource (bool autoConnect = true);
    bool createSoundSource (const char* soundSourceId, bool autoConnect = true);
    bool removeSoundSource (const char* soundSourceId);
    
    void reconnectAllSoundSources()
    {
        GlobalIdPool::instance().for_each_active ([&] (int id)
        {
            for (auto model : getListenerModels())
                model->ConnectSoundSource (std::to_string (id));
        });
    }
    
    //==========================================================================
    bool setHRTF (const char* hrtfFile);
    bool setNFCFilter (const char* nfcFilterFile);
    bool setBRIR (const char* brirFile);
    bool setDirectivityTF (const char* soundSourceID, const char* directivityFile);
    void setDirectivityEnabled (const char* soundSourceID, bool enabled);

    //==========================================================================
    void updateListenerModelParameters (const char* modelId, const ListenerModelParameters* p)
    {
        applyListenerModelParameters (modelId, p);
    }
    
    BRTBase::CBRTManager brtManager;
    std::shared_ptr<BRTBase::CListener> listener;
    
private:
    //==========================================================================
    BRTLibraryWrapper (int sampleRate, int bufferSize);
    ~BRTLibraryWrapper();
    
    void applyListenerModelParameters (const char* modelId, const ListenerModelParameters* p);
    
    std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> getListenerModels();

    // Shared instance
    static std::atomic<BRTLibraryWrapper*> brtInstance;
    
    struct RetiredItem
    {
        BRTLibraryWrapper* ptr;
        int framesLeft;
    };

    static std::mutex retireMutex;
    static std::vector<RetiredItem> retired;
    
    Common::CGlobalParameters globalParameters;
    
    int sampleRate, bufferSize;
    CMonoBuffer<float> outLeftBuffer;
    CMonoBuffer<float> outRightBuffer;
    std::atomic<bool> suspended;
};


//==============================================================================
template <typename ListenerModelType>
inline bool BRTLibraryWrapper::createListenerModel (const char* listenerModelId)
{
    const ScopedSuspendProcessing guard (*this);
    const ScopedManagerSetup managerSetup (brtManager);
    
    if (auto listenerModel = brtManager.CreateListenerModel<ListenerModelType> (listenerModelId))
        return true;
        
    return false;
}

} // namespace BRTUnity
