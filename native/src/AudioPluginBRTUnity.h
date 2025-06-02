
#pragma once
#include "AudioPluginInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*ErrorCallback)(const char*);

UNITY_AUDIODSP_EXPORT_API
void SetErrorCallback (ErrorCallback cb);

UNITY_AUDIODSP_EXPORT_API
void BRTSpatializerResetIfNeeded (int sampleRate, int dspBufferSize);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatialiserSetFloat (int parameter, float value);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatialiserGetFloat (int parameter, float* value);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerCreateListener (const char* listenerId);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerCreateListenerModel (int type, const char* listenerModelId);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerConnectListenerModel (const char* listenerId, const char* listenerModelId);

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

#ifdef __cplusplus
}
#endif
