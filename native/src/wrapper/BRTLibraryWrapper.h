
#pragma once

#define NOMINMAX
#include <algorithm>
#include <cfloat>
#include "BRTLibrary.h"
#include "Parameters.h"
#include "SpatializerRegistry.h"
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

//==============================================================================
class BRTLibraryWrapper
{
    class ScopedSetup
    {
    public:
        explicit ScopedSetup (BRTLibraryWrapper& w);
        ~ScopedSetup();
    private:
        BRTLibraryWrapper& wrapper;
    };
    
public:
    // Audio-thread safe accessor
    static std::shared_ptr<BRTLibraryWrapper> instance() noexcept;

    // Main-thread init/replacement
    static void initOrReplace (int sampleRate, int bufferSize);
    static void destroy();

    bool isCompatible (int sampleRate, int bufferSize) const noexcept;

    void process (float* in, float* out, unsigned int len, int inCh, int outCh) noexcept;
    
    //==========================================================================
    bool createListener (const char* listenerID);
    bool removeListener (const char* listenerID);
    
    template <typename ListenerModelType>
    bool createListenerModel (const char* listenerModelID);
    bool removeListenerModel (const char* listenerModelID);
    bool connectListenerModel (const char* listenerModelID, const char* listenerID);
    
    void clearGraph();
    
    //==========================================================================
    bool createSoundSource (const char* soundSourceID, bool autoConnect = true);
    bool removeSoundSource (const char* soundSourceID);
    bool connectSoundSource (const char* soundSourceID, const char* listenerModelID);
    void reconnectAllSoundSources()
    {
        const ScopedSetup guard (*this);
        
        auto snapshot = SpatializerRegistry::instance().get();

        for (const auto& [id, state] : *snapshot)
        {
            for (auto model : getListenerModels())
            {
                auto idString = std::to_string (id);
                model->DisconnectSoundSource (idString);
                model->ConnectSoundSource (idString);
            }
        };
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
    
    void setListenerTransform (Common::CTransform newTransform)
    {
        if (auto listener = getListener())
            listener->SetListenerTransform (newTransform);
    }
    
protected:
    std::atomic<uint32_t> setupCounter { 0 };
    
private:
    //==========================================================================
    BRTLibraryWrapper (int sampleRate, int bufferSize);
    ~BRTLibraryWrapper();
    
    bool isSuspended() const noexcept;
    void applyListenerModelParameters (const char* modelId, const ListenerModelParameters* p);
    std::shared_ptr<BRTBase::CListener> getListener() const noexcept;
    std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> getListenerModels();

    // Shared instance
    static std::shared_ptr<BRTLibraryWrapper> brtInstance;
    
    BRTBase::CBRTManager brtManager;
    std::shared_ptr<BRTBase::CListener> listener;
    Common::CGlobalParameters globalParameters;
    
    int sampleRate, bufferSize;
    CMonoBuffer<float> outLeftBuffer;
    CMonoBuffer<float> outRightBuffer;
};


//==============================================================================
template <typename ListenerModelType>
inline bool BRTLibraryWrapper::createListenerModel (const char* listenerModelId)
{
    const ScopedSetup guard (*this);
    
    if (auto listenerModel = brtManager.CreateListenerModel<ListenerModelType> (listenerModelId))
        return true;
        
    return false;
}

} // namespace BRTUnity
