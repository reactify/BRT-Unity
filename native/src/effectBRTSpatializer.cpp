/**
 * BRT-Unity: Binaural Spatializer
**/

#include "BRTLibraryWrapper.h"
#include "AudioPluginUtil.h"
#include "AppUtils.h"
#include "Logging.h"
#include "SpatializerRegistry.h"

// DEBUG LOG 
#ifdef UNITY_ANDROID
#define DEBUG_LOG_CAT
#else
#define DEBUG_LOG_FILE_BINSP
#endif

#ifdef DEBUG_LOG_CAT
#include <android/log.h> 
#include <string>
#include <sstream>
#endif

#ifndef _3DTI_AXIS_CONVENTION_UNITY
#error "_3DTI_AXIS_CONVENTION_UNITY is not defined!"
#endif

#ifndef _3DTI_ANGLE_CONVENTION_LISTEN
#error "_3DTI_ANGLE_CONVENTION_LISTEN is not defined!"
#endif

namespace BRTSpatializer
{

using namespace BRTUnity;

//================================================================================
struct EffectData
{
    int sourceID;
    float scaleFactor = 1.0f;
    CMonoBuffer<float> inMonoBuffer;
};

enum Parameter
{
    InstanceId = 0,
    NumParameters,
};

inline int toIndex (Parameter param)
{
    return static_cast<int> (param);
}

int InternalRegisterEffectDefinition (UnityAudioEffectDefinition& definition)
{
    int numparams = 1; // FloatParameter::NumSourceParameters;
    definition.paramdefs = new UnityAudioParameterDefinition[NumParameters];
    AudioPluginUtil::RegisterParameter (definition, "SourceID", "", -1.0f, 1e20f, -1.0f, 1.0f, 1.0f, 0, "Source ID for debug");
    //	RegisterParameter(definition, "HRTFInterp", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableHRTFInterpolation, "HRTF Interpolation method");
    //	RegisterParameter(definition, "MODfarLPF", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableFarDistanceLPF, "Far distance LPF module enabler");
    //	RegisterParameter(definition, "MODDistAtt", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableDistanceAttenuationAnechoic, "Enable distance attenuation for anechoic processing");
    //	RegisterParameter(definition, "MODNFILD", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableNearFieldEffect, "Near distance ILD module enabler");
    //	RegisterParameter(definition, "SpatMode", "", 0.0f, 2.0f, 0.0f, 1.0f, 1.0f, FloatParameter::SpatializationMode, "Spatialization mode (0=High quality, 1=High performance, 2=None)");
    //	RegisterParameter(definition, "EnableReverb", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, FloatParameter::EnableReverbSend, "Enable reverb processing");
    //	RegisterParameter(definition, "RevDistAtt", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableDistanceAttenuationReverb, "Enable distance attenuation for reverb processing");
        //Sample Rate and BufferSize
	definition.flags |= UnityAudioEffectDefinitionFlags_IsSpatializer;
	return numparams;
}

//==============================================================================
static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
DistanceAttenuationCallback (UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut)
{
	*attenuationOut = attenuationIn;
	return UNITY_AUDIODSP_OK;
}

//==============================================================================
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
CreateCallback (UnityAudioEffectState* state)
{
    BRTLibraryWrapper::initOrReplace (state->samplerate, state->dspbuffersize);
    
    auto brt = BRTLibraryWrapper::instance();

    int instanceId = GlobalIdPool::instance().acquire();

    SpatializerRegistry::instance().set (instanceId, {});
    brt->createSoundSource (std::to_string(instanceId).c_str());
    BRT_Log (0, "[effectBRTSpatializer] Created Spatializer Source " + std::to_string (instanceId));

    EffectData* data = new EffectData;
    data->sourceID = instanceId;
    data->inMonoBuffer.resize (state->dspbuffersize);

    state->effectdata = data;
    state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;

	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
ReleaseCallback (UnityAudioEffectState* state)
{
    if (auto* data = state->GetEffectData<EffectData>())
    {
        auto brt = BRTLibraryWrapper::instance();

        if (brt)
        {
            SpatializerRegistry::instance().erase (data->sourceID);
            brt->removeSoundSource (std::to_string (data->sourceID).c_str());
        }

        GlobalIdPool::instance().release (data->sourceID);

        delete data;
    }
    
	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
SetFloatParameterCallback (UnityAudioEffectState* state, int index, float value)
{
    BRT_Log (1, "[effectBRTSpatializer] SetFloatParameterCallback not supported");
    return UNITY_AUDIODSP_ERR_UNSUPPORTED;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
GetFloatParameterCallback (UnityAudioEffectState* state, int index, float* value, char* valuestr)
{
    EffectData* data = state->GetEffectData<EffectData>();
    
	if (valuestr != NULL)
	{
		valuestr[0] = '\0';
	}
    
    BRT_Log (0, "[effectBRTSpatializer] Getting float parameter " + std::to_string (index));

	if (value != NULL)
	{
		switch (static_cast<Parameter> (index))
		{
            case InstanceId:
                BRT_Log (0, "[effectBRTSpatializer] Returning sourceID " + std::to_string (data->sourceID));
                *value = (float) data->sourceID;
                break;
            default:
                return UNITY_AUDIODSP_ERR_UNSUPPORTED;
		}
	}
	return UNITY_AUDIODSP_OK;
}

int UNITY_AUDIODSP_CALLBACK
GetFloatBufferCallback (UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
{
	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
ProcessCallback (UnityAudioEffectState* state, float* inbuffer, float* outbuffer,
                 unsigned int length, int inchannels, int outchannels)
{
    auto brt = BRTLibraryWrapper::instance();

    if (inchannels != 2 || outchannels != 2 ||
      ! brt || ! brt->isCompatible (state->samplerate, state->dspbuffersize))
    {
        memcpy (outbuffer, inbuffer, length * outchannels * sizeof (float));
        return UNITY_AUDIODSP_OK;
    }
 
    auto* data = state->GetEffectData<EffectData>();

    for (size_t i = 0; i < length; ++i)
        data->inMonoBuffer[i] = (inbuffer[i * 2] + inbuffer[i * 2 + 1]) * 0.5f;
    
    SpatializerState s;
    s.sourceTransform = ComputeSourceTransformFromMatrix (state->spatializerdata->sourcematrix, data->scaleFactor);
    s.buffer = data->inMonoBuffer;
    SpatializerRegistry::instance().set (data->sourceID, std::move (s));
    
    brt->setListenerTransform (ComputeListenerTransformFromMatrix (state->spatializerdata->listenermatrix, data->scaleFactor));

    for (size_t i = 0; i < (size_t) length * std::max (inchannels, outchannels); ++i)
        outbuffer[i] = 0.0f;

	return UNITY_AUDIODSP_OK;
}

}
