
#include "AudioPluginBRTUnity.h"
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"

using namespace BRTUnity;

inline void WriteLog (std::string logText)
{
    std::cerr << logText << std::endl;
}

ErrorCallback g_errorCallback = nullptr;

void RaiseError (const char* msg)
{
    if (g_errorCallback)
        g_errorCallback (msg);
}

void SetErrorCallback (ErrorCallback cb)
{
    g_errorCallback = cb;
}

void BRTSpatializerResetIfNeeded (int sampleRate, int dspBufferSize)
{
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        brtInstance->initOrReplace (sampleRate, dspBufferSize);
}

// TODO
bool BRTSpatialiserSetFloat (int parameter, float value)
{
    return false;
}

// TODO
bool BRTSpatialiserGetFloat (int parameter, float* value)
{
    return false;
}

bool BRTSpatializerCreateListener (const char* listenerId)
{
    WriteLog ("BRT: Creating Listener: " +  std::string (listenerId));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("[BRTSpatializerCreateListener]: No Spatializer exists");
        return false;
    }
    
    auto& brtManager = brtInstance->brtManager;
    const ScopedManagerSetup sm (brtManager);
    
    if (auto listener = brtManager.CreateListener<BRTBase::CListener> (listenerId))
    {
        brtInstance->listener = listener;
        return true;
    }
    
    return false;
}

bool BRTSpatializerCreateListenerModel (int type, const char* listenerModelId)
{
    WriteLog ("BRT: Creating Listener Model: " +  std::string (listenerModelId));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("[BRTSpatializerCreateListener]: No Spatializer exists");
        return false;
    }
    
    auto& brtManager = brtInstance->brtManager;
    
    const ScopedManagerSetup sm (brtManager);
    
    using namespace BRTListenerModel;
    std::shared_ptr<CListenerModelBase> listenerModel = nullptr;
    
    switch (type)
    {
        case 0:
            listenerModel = brtManager.CreateListenerModel<CListenerHRTFModel> (listenerModelId);
            break;
        case 1:
            listenerModel = brtManager.CreateListenerModel<CListenerAmbisonicEnvironmentBRIRModel> (listenerModelId);
            break;
        default:
            listenerModel = brtManager.CreateListenerModel<CListenerHRTFModel> (listenerModelId);
            break;
    }
    
    if (listenerModel == nullptr)
    {
        WriteLog ("BRT: Error creating listener model");
        return false;
    }
    
    return true;
}

bool BRTSpatializerConnectListenerModel (const char* listenerId, const char* listenerModelId)
{
    WriteLog ("BRT: Connecting Listener Model: " +  std::string (listenerModelId) + " to listener: " + std::string (listenerId));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("BRT Error: No spatializer instance found");
        return false;
    }
    
    auto& brtManager = brtInstance->brtManager;
    
    if (auto listener = brtManager.GetListener (listenerId))
    {
        const ScopedManagerSetup sm (brtManager);
        
        if (! listener->ConnectListenerModel (listenerModelId))
        {
            RaiseError ("BRT: Error connecting listener model");
            return false;
        }
        
        WriteLog ("Connected listener model " + std::string (listenerModelId));
        
        return true;
    }
    
    RaiseError ("BRT: No listener found");
    
    return false;
}

bool BRTSpatializerLoadHRTF (const char* hrtfFile) // TODO: return index?
{
    WriteLog ("BRT: Loading HRTF " +  std::string (hrtfFile));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("BRT Error: No spatializer instance found");
        return false;
    }
    
    auto hrtf = std::make_shared<BRTServices::CHRTF>();
    
    if (! AppUtils::LoadHRTFSofaFile (hrtfFile, hrtf))
    {
        RaiseError ("BRT: Error loading SOFA HRTF");
        return false;
    }
    
    if (auto listener = brtInstance->listener)
        return listener->SetHRTF (hrtf);

    WriteLog ("BRT: Error setting HRTF");
    
    return false;
}

bool BRTSpatializerLoadNearFieldCompensationFilter (const char* nfcFilterFile)
{
    WriteLog ("BRT: Loading NFC Filter " +  std::string (nfcFilterFile));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("BRT Error: No spatializer instance found");
        return false;
    }
    
    auto sosFilter = std::make_shared<BRTServices::CSOSFilters>();
    
    if (! AppUtils::LoadNearFieldSOSFilter (nfcFilterFile, sosFilter))
    {
        RaiseError ("BRT: Error loading SOFA NFC file");
        return false;
    }
    
    if (auto listener = brtInstance->listener)
        return listener->SetNearFieldCompensationFilters (sosFilter);

    WriteLog ("BRT: Error setting NFC Filter");
    
    return true;
}

bool BRTSpatializerLoadBRIR (const char* brirFile)
{
    WriteLog ("BRT: Loading BRIR " +  std::string (brirFile));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("BRT Error: No spatializer instance found");
        return false;
    }
    
    auto brir = std::make_shared<BRTServices::CHRBRIR>();
    
    if (! AppUtils::LoadBRIRSofaFile (brirFile, brir, 0,0,0,0))
    {
        RaiseError ("BRT: Error loading SOFA BRIR");
        return false;
    }
    
    if (auto listener = brtInstance->listener)
        return listener->SetHRBRIR (brir);

    RaiseError ("BRT: Error setting BRIR");
    
    return false;
}

bool BRTSpatializerCreateSoundSource (const char* sourceId)
{
    auto sourceIDStr = std::string (sourceId);
    WriteLog ("BRT: Creating sound source: " + sourceIDStr);
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        WriteLog ("BRT Error: No spatializer instance found");
        return false;
    }
    
    auto& brtManager = brtInstance->brtManager;
    const ScopedManagerSetup sm (brtManager);

    auto soundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceSimpleModel> (sourceIDStr);

    if (soundSource == nullptr)
    {
        WriteLog ("BRT: Error creating sound source: " + sourceIDStr);
        return false;
    }
    
    return true;
}

bool BRTSpatializerConnectSoundSource (const char* soundSourceID, const char* listenerModelID)
{
    WriteLog ("BRT: Connecting sound source: " + std::string (soundSourceID));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        WriteLog ("BRT Error: No spatializer instance found");
        return false;
    }
    
    if (brtInstance->listener)
    {
        WriteLog ("BRT: Listener exists. Finding listener model: " + std::string (listenerModelID));
     
        auto& brtManager = brtInstance->brtManager;
        
        const ScopedManagerSetup sm (brtManager);
        
        if (auto listenerModel = brtManager.GetListenerModel<BRTListenerModel::CListenerModelBase> (listenerModelID))
        {
            return listenerModel->ConnectSoundSource (soundSourceID);
        }
    }

    RaiseError ("Error connecting sound source. No listener found");
    
    return false;
}
