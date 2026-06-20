
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
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->createListener (listenerID);
    
    BRT_Log (2, "[BRTCreateListener]: No BRTLibrary instance found");
    return false;
}

bool BRTRemoveListener (const char* listenerID)
{
    BRT_Log (0, "Removing Listener: " +  std::string (listenerID));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->removeListener (listenerID);
    
    BRT_Log (2, "[BRTRemoveListener]: No BRTLibrary instance found");
    return false;
}

bool BRTCreateListenerModel (int type, const char* listenerModelId)
{
    BRT_Log (0, "Creating Listener Model: " +  std::string (listenerModelId) + " of type: " + std::to_string (type));
    
    auto brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        BRT_Log (2, "[BRTCreateListenerModel] No BRTLibrary instance found");
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

bool BRTRemoveListenerModel (const char* listenerModelID)
{
    BRT_Log (0, "Removing Listener Model: " +  std::string (listenerModelID));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->removeListenerModel (listenerModelID);
    
    BRT_Log (2, "[BRTRemoveListenerModel] No BRTLibrary instance found");
    return false;
}

bool BRTConnectListenerModel (const char* listenerModelID, const char* listenerID)
{
    BRT_Log (0, "Connecting Listener Model: " +  std::string (listenerModelID) + " to listener: " + std::string (listenerID));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->connectListenerModel (listenerModelID, listenerID);
    
    BRT_Log (2, "[BRTConnectListenerModel] No BRTLibrary instance found");
    return false;
}

bool BRTCreateEnvironmentModel (int type, const char* environmentModelId)
{
    BRT_Log (0, "Creating Environment Model: " +  std::string (environmentModelId) + " of type: " + std::to_string (type));
    
    auto brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        BRT_Log (2, "[BRTCreateEnvironmentModel] No BRTLibrary instance found");
        return false;
    }
    
    using namespace BRTEnvironmentModel;
    
    switch (type)
    {
        case 0:
            return brtInstance->createEnvironmentModel<CFreeFieldEnvironmentModel> (environmentModelId);
        case 1:
            return brtInstance->createEnvironmentModel<CSDNEnvironmentModel> (environmentModelId);
        default:
            return brtInstance->createEnvironmentModel<CFreeFieldEnvironmentModel> (environmentModelId);
    }
}

bool BRTRemoveEnvironmentModel (const char* environmentModelId)
{
    BRT_Log (0, "Removing Environment Model: " +  std::string (environmentModelId));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->removeEnvironmentModel (environmentModelId);
    
    BRT_Log (2, "[BRTRemoveEnvironmentModel] No BRTLibrary instance found");
    return false;
}

bool BRTConnectEnvironmentModel (const char* environmentModelId, const char* listenerModelId)
{
    BRT_Log (0, "Connecting Environment Model: " +  std::string (environmentModelId) + " to listener model: " + std::string (listenerModelId));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->connectEnvironmentModel (environmentModelId, listenerModelId);
    
    BRT_Log (2, "[BRTConnectEnvironmentModel] No BRTLibrary instance found");
    return false;
}

void BRTClearGraph()
{
    BRT_Log (0, "BRTClearGraph");
    BRTLibraryWrapper::clearGraph();
}

void BRTSetListenerModelParameters (const char* listenerModelId, const BRTUnity::ListenerModelParameters* params)
{
    BRT_Log (0, "BRTSetListenerModelParameters");
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->updateListenerModelParameters (listenerModelId, params);
    
    BRT_Log (2, "[BRTSetListenerModelParameters] No BRTLibrary instance found");
}

bool BRTSpatializerLoadHRTF (const char* hrtfFile) // TODO: return index?
{
    BRT_Log (0, "Loading HRTF " +  std::string (hrtfFile));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setHRTF (hrtfFile);
    
    BRT_Log (2, "[BRTSpatializerLoadHRTF] No BRTLibrary instance found");
    return false;
}

bool BRTSpatializerLoadNearFieldCompensationFilter (const char* nfcFilterFile)
{
    BRT_Log (0, "Loading NFC Filter " +  std::string (nfcFilterFile));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setNFCFilter (nfcFilterFile);
    
    BRT_Log (2, "[BRTSpatializerLoadNearFieldCompensationFilter] No BRTLibrary instance found");
    return false;
}

bool BRTSpatializerLoadBRIR (const char* brirFile)
{
    BRT_Log (0, "Loading BRIR " +  std::string (brirFile));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setBRIR (brirFile);

    BRT_Log (2, "[BRTSpatializerLoadBRIR] No BRTLibrary instance found");
    return false;
}

bool BRTLoadSourceDirectivityTF (const char* soundSourceID, const char* directivityFile)
{
    BRT_Log (0, "Loading DirectivityTF " +  std::string (directivityFile));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setDirectivityTF (soundSourceID, directivityFile);
    
    BRT_Log (2, "[BRTLoadSourceDirectivityTF] No BRTLibrary instance found");
    return false;
}

void BRTSetSourceDirectivityEnabled (const char* soundSourceID, bool enabled)
{
    BRT_Log (0, "Setting Directivity enabled " +  std::string (enabled ? "true" : " false"));
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->setDirectivityEnabled (soundSourceID, enabled);
    
    BRT_Log (2, "[BRTSetSourceDirectivityEnabled] No BRTLibrary instance found");
}

bool BRTCreateSoundSource (const char* sourceId)
{
    auto sourceIDStr = std::string (sourceId);
    BRT_Log (0, "Creating sound source: " + sourceIDStr);
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->createSoundSource (sourceId);
    
    BRT_Log (2, "[BRTCreateSoundSource] No BRTLibrary instance found");
    return false;
}

bool BRTConnectSoundSource (const char* soundSourceID, const char* listenerModelID)
{
    BRT_Log (0, "Connecting sound source: " + std::string (soundSourceID));
    
    if (auto brt = BRTLibraryWrapper::instance())
        return brt->connectSoundSource (soundSourceID, listenerModelID);
    
    BRT_Log (2, "[BRTConnectSoundSource] No BRTLibrary instance found");
    return false;
}

void BRTReconnectAllSoundSources()
{
    BRT_Log (0, "Re-connecting sound sources");
    
    if (auto brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->reconnectAllSoundSources();
    
    BRT_Log (2, "[BRTReconnectAllSoundSources] No BRTLibrary instance found");
    return false;
}
