
#include "AudioPluginBRTUnity.h"
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"
#include "Logging.h"

using namespace BRTUnity;

void BRTSpatializerResetIfNeeded (int sampleRate, int dspBufferSize)
{
    BRT_Log (0, "BRTSpatializerResetIfNeeded");
    BRTLibraryWrapper::initOrReplace (sampleRate, dspBufferSize);
}

void BRTSpatializerDestroy()
{
    BRT_Log (0, "BRTSpatializerDestroy");
    BRTLibraryWrapper::destroy();
}

bool BRTCreateListener (const char* listenerID)
{
    BRT_Log (0, "Creating Listener: " +  std::string (listenerID));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->createListener (listenerID);
    
    BRT_Log (2, "[BRTCreateListener]: No Spatializer exists");
    return false;
}

bool BRTRemoveListener (const char* listenerID)
{
    BRT_Log (0, "Removing Listener: " +  std::string (listenerID));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->removeListener (listenerID);
    
    BRT_Log (2, "[BRTRemoveListener]: No Spatializer exists");
    return false;
}

bool BRTSpatializerCreateListenerModel (int type, const char* listenerModelId)
{
    BRT_Log (0, "Creating Listener Model: " +  std::string (listenerModelId) + " of type: " + std::to_string (type));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        BRT_Log (2, "[BRTSpatializerCreateListener]: No Spatializer exists");
        return false;
    }
    
    using namespace BRTListenerModel;
    
    switch (type)
    {
        case 0:
            return brtInstance->createListenerModel<CListenerDirectHRTFConvolutionModel> (listenerModelId);
        case 1:
            return brtInstance->createListenerModel<CListenerAmbisonicVirtualLoudspeakersModel> (listenerModelId);
        case 2:
            return brtInstance->createListenerModel<CListenerDirectBRIRConvolutionModel> (listenerModelId);
        case 3:
            return brtInstance->createListenerModel<CListenerAmbisonicReverberantVirtualLoudspeakersModel> (listenerModelId);
        default:
            return brtInstance->createListenerModel<CListenerDirectHRTFConvolutionModel> (listenerModelId);
    }
}

bool BRTSpatializerConnectListenerModel (const char* listenerId, const char* listenerModelId)
{
    BRT_Log (0, "Connecting Listener Model: " +  std::string (listenerModelId) + " to listener: " + std::string (listenerId));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        BRT_Log (2, "BRT Error: No spatializer instance found");
        return false;
    }
    
    auto& brtManager = brtInstance->brtManager;
    
    if (auto listener = brtManager.GetListener (listenerId))
    {
        const ScopedManagerSetup sm (brtManager);
        
        if (! listener->ConnectListenerModel (listenerModelId))
        {
            BRT_Log (2, "BRT: Error connecting listener model");
            return false;
        }
        
        BRT_Log (0,"Connected listener model " + std::string (listenerModelId));
        
        return true;
    }
    
    BRT_Log (2, "BRT: No listener found");
    
    return false;
}

void BRTSetListenerModelParameters (const char* listenerModelId, const BRTUnity::ListenerModelParameters* params)
{
    BRT_Log (0, "BRTSetListenerModelParameters");
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateListenerModelParameters (listenerModelId, params);
    }
}

bool BRTSpatializerLoadHRTF (const char* hrtfFile) // TODO: return index?
{
    BRT_Log (0, "Loading HRTF " +  std::string (hrtfFile));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setHRTF (hrtfFile);
    
    BRT_Log (2, "BRT Error: No spatializer instance found");
    return false;
}

bool BRTSpatializerLoadNearFieldCompensationFilter (const char* nfcFilterFile)
{
    BRT_Log (0, "Loading NFC Filter " +  std::string (nfcFilterFile));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setNFCFilter (nfcFilterFile);
    
    BRT_Log (2, "BRT Error: No spatializer instance found");
    return false;
}

bool BRTSpatializerLoadBRIR (const char* brirFile)
{
    BRT_Log (0, "Loading BRIR " +  std::string (brirFile));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setBRIR (brirFile);

    BRT_Log (2, "Error setting BRIR");
    return false;
}

bool BRTLoadSourceDirectivityTF (const char* soundSourceID, const char* directivityFile)
{
    BRT_Log (0, "Loading DirectivityTF " +  std::string (directivityFile));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setDirectivityTF (soundSourceID, directivityFile);
    
    BRT_Log (2, "BRT Error: No spatializer instance found");
    return false;
}

void BRTSetSourceDirectivityEnabled (const char* soundSourceID, bool enabled)
{
    BRT_Log (0, "Setting Directivity enabled " +  std::string (enabled ? "true" : " false"));
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setDirectivityEnabled (soundSourceID, enabled);
    
    BRT_Log (2, "BRT Error: No spatializer instance found");
}

bool BRTCreateSoundSource (const char* sourceId)
{
    auto sourceIDStr = std::string (sourceId);
    BRT_Log (0, "Creating sound source: " + sourceIDStr);
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->createSoundSource (sourceId);
    
    BRT_Log (2, "BRT Error: No spatializer instance found");
    return false;
}

bool BRTConnectSoundSource (const char* soundSourceID, const char* listenerModelID)
{
    BRT_Log (0, "Connecting sound source: " + std::string (soundSourceID));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        BRT_Log (2, "No spatializer instance found");
        return false;
    }
    
    if (brtInstance->listener)
    {
        BRT_Log (0, "Listener exists. Finding listener model: " + std::string (listenerModelID));
     
        auto& brtManager = brtInstance->brtManager;
        
        const ScopedManagerSetup sm (brtManager);
        
        if (auto listenerModel = brtManager.GetListenerModel<BRTListenerModel::CListenerModelBase> (listenerModelID))
        {
            return listenerModel->ConnectSoundSource (soundSourceID);
        }
    }

    BRT_Log (2, "Error connecting sound source. No listener found");
    
    return false;
}

void BRTReconnectAllSoundSources()
{
    BRT_Log (0, "Re-connecting sound sources");
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        brtInstance->reconnectAllSoundSources();
    
    BRT_Log (2, "Error reconnecting sound sources. No spatializer instance found");
    return false;
}
