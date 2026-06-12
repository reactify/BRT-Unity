
#pragma once

#include "AudioPluginInterface.h"
#include "Parameters.h"

#ifdef __cplusplus
extern "C" {
#endif

UNITY_AUDIODSP_EXPORT_API
void BRTSpatializerResetIfNeeded (int sampleRate, int dspBufferSize);

UNITY_AUDIODSP_EXPORT_API
void BRTSpatializerDestroy();

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerCreateListener (const char* listenerId);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerCreateListenerModel (int type, const char* listenerModelId);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerConnectListenerModel (const char* listenerId, const char* listenerModelId);

//struct BRTListenerModelSettings
//{
//    bool enabled;
//    bool spatializationEnabled;
//    bool itdEnabled;
//    bool parallaxCorrectionEnabled;
//    bool nearFieldEffectEnabled;
//};

UNITY_AUDIODSP_EXPORT_API
void BRTSetListenerModelParameters (const char* listenerModelId, const BRTUnity::ListenerModelParameters* params);

//UNITY_AUDIODSP_EXPORT_API
//bool BRTSpatializerSetListenerModelSettings (const char* listenerModelId, const BRTListenerModelSettings& settings);
//UNITY_AUDIODSP_EXPORT_API
//bool BRTSpatializerSetListenerModelEnabled (const char* listenerModelId, bool enabled);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTSpatializerSetListenerModelSpatializationEnabled (const char* listenerModelId, bool enabled);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTSpatializerSetListenerModelITDEnabled (const char* listenerModelId, bool enabled);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTSpatializerSetListenerModelParallaxCorrectionEnabled (const char* listenerModelId, bool enabled);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTSpatializerSetListenerModelNearFieldEffectEnabled (const char* listenerModelId, bool enabled);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerLoadHRTF (const char* hrtfFile); // TODO: return index?

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerLoadNearFieldCompensationFilter (const char* nfcFilterFile);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerLoadBRIR (const char* brirFile);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerCreateSoundSource (const char* sourceId);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerConnectSoundSource (const char* soundSourceID, const char* listenerModelID);

//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetBypassed (bool bypass);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetSpatializationEnabled (bool bypass);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetInterpolationEnabled (bool bypass);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetITDSimulationEnabled (bool bypass);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetNearFieldEffectEnabled (bool bypass);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetParallaxCorrectionEnabled (bool bypass);
//
//UNITY_AUDIODSP_EXPORT_API
//bool BRTManagerSetDistanceAttenuationEnabled (bool bypass);

#ifdef __cplusplus
}
#endif
