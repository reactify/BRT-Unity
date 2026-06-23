
#pragma once

#define NOMINMAX
#include <algorithm>
#include <cfloat>
#include <stack>
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
    
    template <typename EnvironmentModelType>
    bool createEnvironmentModel (const char* environmentModelID);
    bool removeEnvironmentModel (const char* environmentModelID);
    bool connectEnvironmentModel (const char* environmentModelID, const char* listenerModelID);
    
    static void clearGraph();
    
    //==========================================================================
    bool createSoundSource (const char* soundSourceID, bool autoConnect = true);
    bool removeSoundSource (const char* soundSourceID);
    bool connectSoundSource (std::string soundSourceID, std::string listenerModelID);
    bool disconnectSoundSource (std::string soundSourceID, std::string listenerModelID);
    void reconnectAllSoundSources()
    {
        auto snapshot = SpatializerRegistry::instance().get();

        for (const auto& [id, state] : *snapshot)
        {
            auto idString = std::to_string (id);
            
            if (! brtManager.GetSoundSource (idString))
                createSoundSource (idString.c_str(), false);
            
            for (auto listenerModel : getListenerModels())
            {
                auto listenerModelID = listenerModel->GetModelID();
                
                for (auto environmentModel : getEnvironmentModels())
                {
                    auto environmentModelID = environmentModel->GetModelID();
                    
                    if (environmentModel->GetIDEntryPoint("listenerModelID")->GetData() == listenerModelID)
                    {
                        disconnectSoundSource (idString, environmentModelID);
                        connectSoundSource (idString, environmentModelID);
                        continue;
                    }
                }
                
                disconnectSoundSource (idString, listenerModelID);
                connectSoundSource (idString, listenerModelID);
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
    
    void updateEnvironmentModelParameters (const char* modelId, const EnvironmentModelParameters* p)
    {
        applyEnvironmentModelParameters (modelId, p);
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
    void applyEnvironmentModelParameters (const char* modelId, const EnvironmentModelParameters* p);
    std::shared_ptr<BRTBase::CListener> getListener() const noexcept;
    std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> getListenerModels();
    std::vector<std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase>> getEnvironmentModels();

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
    
    if (auto model = brtManager.CreateListenerModel<ListenerModelType> (listenerModelId))
        return true;
        
    return false;
}

//==============================================================================
template <typename EnvironmentModelType>
inline bool BRTLibraryWrapper::createEnvironmentModel (const char* environmentModelId)
{
    const ScopedSetup guard (*this);
    
    if (auto model = brtManager.CreateEnvironment<EnvironmentModelType> (environmentModelId))
        return true;
        
    return false;
}

} // namespace BRTUnity
