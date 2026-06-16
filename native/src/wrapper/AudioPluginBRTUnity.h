
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

UNITY_AUDIODSP_EXPORT_API
void BRTSetListenerModelParameters (const char* listenerModelId, const BRTUnity::ListenerModelParameters* params);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerLoadHRTF (const char* hrtfFile); // TODO: return index?

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerLoadNearFieldCompensationFilter (const char* nfcFilterFile);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerLoadBRIR (const char* brirFile);

UNITY_AUDIODSP_EXPORT_API
bool BRTLoadSourceDirectivityTF (const char* soundSourceID, const char* directivityFile);

UNITY_AUDIODSP_EXPORT_API
void BRTSetSourceDirectivityEnabled (const char* soundSourceID, bool enabled);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerCreateSoundSource (const char* sourceId);

UNITY_AUDIODSP_EXPORT_API
bool BRTSpatializerConnectSoundSource (const char* soundSourceID, const char* listenerModelID);

#ifdef __cplusplus
}
#endif
