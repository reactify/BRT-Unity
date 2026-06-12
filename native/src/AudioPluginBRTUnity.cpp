
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

bool BRTSpatializerCreateListener (const char* listenerId)
{
    BRT_Log (0, "Creating Listener: " +  std::string (listenerId));
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    
    if (brtInstance == nullptr)
    {
        BRT_Log (2, "[BRTSpatializerCreateListener]: No Spatializer exists");
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
            return brtInstance->createListenerModel<CListenerDirectBRIRConvolutionModel> (listenerModelId);
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

bool BRTSpatializerCreateSoundSource (const char* sourceId)
{
    auto sourceIDStr = std::string (sourceId);
    BRT_Log (0, "Creating sound source: " + sourceIDStr);
    
    if (auto* brtInstance = BRTLibraryWrapper::instance())
        return brtInstance->createSoundSource (sourceId);
    
    BRT_Log (2, "BRT Error: No spatializer instance found");
    return false;
}

bool BRTSpatializerConnectSoundSource (const char* soundSourceID, const char* listenerModelID)
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

//bool BRTManagerSetBypassed (bool bypass)
//{
//    BRT_Log (0, "Setting Bypassed " + std::string (bypass ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.bypassed = bypass;
//        });
//    }
//}
//
//bool BRTManagerSetSpatializationEnabled (bool enabled)
//{
//    BRT_Log (0, "Setting Spatialization Enabled " + std::string (enabled ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.spatializationEnabled = enabled;
//        });
//    }
//}
//
//bool BRTManagerSetInterpolationEnabled (bool enabled)
//{
//    BRT_Log (0, "Setting Interpolation Enabled " + std::string (enabled ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.interpolationEnabled = enabled;
//        });
//    }
//}
//
//bool BRTManagerSetITDSimulationEnabled (bool enabled)
//{
//    BRT_Log (0, "Setting ITD Simulation Enabled " + std::string (enabled ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.itdSimulationEnabled = enabled;
//        });
//    }
//}
//
//bool BRTManagerSetNearFieldEffectEnabled (bool enabled)
//{
//    BRT_Log (0, "Setting Near Field Effect Enabled " + std::string (enabled ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.nearFieldEffectEnabled = enabled;
//        });
//    }
//}
//
//bool BRTManagerSetParallaxCorrectionEnabled (bool enabled)
//{
//    BRT_Log (0, "Setting Parallax Correction Enabled " + std::string (enabled ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.parallaxCorrectionEnabled = enabled;
//        });
//    }
//}
//
//bool BRTManagerSetDistanceAttenuationEnabled (bool enabled)
//{
//    BRT_Log (0, "Setting Distance Attenuation Enabled " + std::string (enabled ? "1" : "0"));
//    if (auto* brtInstance = BRTLibraryWrapper::instance())
//    {
//        brtInstance->updateParameters ([=] (auto& p) {
//            p.distanceAttenuationEnabled = enabled;
//        });
//    }
//}
