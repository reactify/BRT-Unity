
#include "AudioPluginBRTUnity.h"
#include "BRTLibraryWrapper.h"
#include "AppUtils.h"

using namespace BRTUnity;

inline void WriteLog (std::string logText)
{
    std::cerr << "[BRT NATIVE] " << logText << std::endl;
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
    BRTLibraryWrapper::initOrReplace (sampleRate, dspBufferSize);
}

// TODO
bool BRTSpatializerSetFloat (int parameter, float value)
{
    return false;
}

// TODO
bool BRTSpatializerGetFloat (int parameter, float* value)
{
    return false;
}

bool BRTSpatializerCreateListener (const char* listenerId)
{
    WriteLog ("Creating Listener: " +  std::string (listenerId));
    
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
    WriteLog ("Creating Listener Model: " +  std::string (listenerModelId));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        RaiseError ("[BRTSpatializerCreateListener]: No Spatializer exists");
        return false;
    }
    
    using namespace BRTListenerModel;
    
    switch (type)
    {
        case 0:
            return brtInstance->createListenerModel<CListenerHRTFModel> (listenerModelId);
        case 1:
            return brtInstance->createListenerModel<CListenerAmbisonicEnvironmentBRIRModel> (listenerModelId);
        default:
            return brtInstance->createListenerModel<CListenerHRTFModel> (listenerModelId);
    }
}

bool BRTSpatializerConnectListenerModel (const char* listenerId, const char* listenerModelId)
{
    WriteLog ("Connecting Listener Model: " +  std::string (listenerModelId) + " to listener: " + std::string (listenerId));
    
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

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerSetListenerModelEnabled (const char* listenerModelId, bool enabled)
{
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        using namespace BRTListenerModel;
        
        if (auto listenerModel = brtInstance->brtManager.GetListenerModel<CListenerModelBase> (listenerModelId))
        {
            if (enabled)
                listenerModel->EnableModel();
            else
                listenerModel->DisableModel();
        }
    }
}

bool BRTSpatializerLoadHRTF (const char* hrtfFile) // TODO: return index?
{
    WriteLog ("Loading HRTF " +  std::string (hrtfFile));
    
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

    WriteLog ("Error setting HRTF");
    
    return false;
}

bool BRTSpatializerLoadNearFieldCompensationFilter (const char* nfcFilterFile)
{
    WriteLog ("Loading NFC Filter " +  std::string (nfcFilterFile));
    
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

    WriteLog ("Error setting NFC Filter");
    
    return true;
}

bool BRTSpatializerLoadBRIR (const char* brirFile)
{
    WriteLog ("Loading BRIR " +  std::string (brirFile));
    
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
    WriteLog ("Creating sound source: " + sourceIDStr);
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->createSoundSource (sourceId);
    
    WriteLog ("BRT Error: No spatializer instance found");
    return false;
}

bool BRTSpatializerConnectSoundSource (const char* soundSourceID, const char* listenerModelID)
{
    WriteLog ("Connecting sound source: " + std::string (soundSourceID));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        WriteLog ("BRT Error: No spatializer instance found");
        return false;
    }
    
    if (brtInstance->listener)
    {
        WriteLog ("Listener exists. Finding listener model: " + std::string (listenerModelID));
     
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

bool BRTManagerSetBypassed (bool bypass)
{
    WriteLog ("Setting Bypassed " + std::string (bypass ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.bypassed = bypass;
        });
    }
}

bool BRTManagerSetSpatializationEnabled (bool enabled)
{
    WriteLog ("Setting Spatialization Enabled " + std::string (enabled ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.spatializationEnabled = enabled;
        });
    }
}

bool BRTManagerSetInterpolationEnabled (bool enabled)
{
    WriteLog ("Setting Interpolation Enabled " + std::string (enabled ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.interpolationEnabled = enabled;
        });
    }
}

bool BRTManagerSetITDSimulationEnabled (bool enabled)
{
    WriteLog ("Setting ITD Simulation Enabled " + std::string (enabled ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.itdSimulationEnabled = enabled;
        });
    }
}

bool BRTManagerSetNearFieldEffectEnabled (bool enabled)
{
    WriteLog ("Setting Near Field Effect Enabled " + std::string (enabled ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.nearFieldEffectEnabled = enabled;
        });
    }
}

bool BRTManagerSetParallaxCorrectionEnabled (bool enabled)
{
    WriteLog ("Setting Parallax Correction Enabled " + std::string (enabled ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.parallaxCorrectionEnabled = enabled;
        });
    }
}

bool BRTManagerSetDistanceAttenuationEnabled (bool enabled)
{
    WriteLog ("Setting Distance Attenuation Enabled " + std::string (enabled ? "1" : "0"));
    if (auto* brtInstance = BRTLibraryWrapper::instance())
    {
        brtInstance->updateParameters ([=] (auto& p) {
            p.distanceAttenuationEnabled = enabled;
        });
    }
}
