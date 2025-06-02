/**
 * BRT-Unity: Binaural Spatializer
**/

#include "BRTLibraryWrapper.h"
#include "AudioPluginUtil.h"
#include "AppUtils.h"

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

inline void WriteLog (std::string logText)
{
    std::cerr << logText << std::endl;
}

int InternalRegisterEffectDefinition (UnityAudioEffectDefinition& definition)
{
    int numparams = 1; // FloatParameter::NumSourceParameters;
    definition.paramdefs = new UnityAudioParameterDefinition[NumParameters];
    RegisterParameter (definition, "SourceID", "", -1.0f, 1e20f, -1.0f, 1.0f, 1.0f, 0, "Source ID for debug");
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

Common::CTransform ComputeListenerTransformFromMatrix(float* listenerMatrix, float scale)
{
	// SET LISTENER POSITION

	// Inverted 4x4 listener matrix, as provided by Unity
	float L[16];
	for (int i = 0; i < 16; i++)
		L[i] = listenerMatrix[i];

	float listenerpos_x = -(L[0] * L[12] + L[1] * L[13] + L[2] * L[14]) * scale;	// From Unity documentation, if listener is rotated
	float listenerpos_y = -(L[4] * L[12] + L[5] * L[13] + L[6] * L[14]) * scale;	// From Unity documentation, if listener is rotated
	float listenerpos_z = -(L[8] * L[12] + L[9] * L[13] + L[10] * L[14]) * scale;	// From Unity documentation, if listener is rotated
	//float listenerpos_x = -L[12] * scale;	// If listener is not rotated
	//float listenerpos_y = -L[13] * scale;	// If listener is not rotated
	//float listenerpos_z = -L[14] * scale;	// If listener is not rotated
	Common::CTransform listenerTransform;
	listenerTransform.SetPosition(Common::CVector3(listenerpos_x, listenerpos_y, listenerpos_z));

	// SET LISTENER ORIENTATION

	//float w = 2 * sqrt(1.0f + L[0] + L[5] + L[10]);
	//float qw = w / 4.0f;
	//float qx = (L[6] - L[9]) / w;
	//float qy = (L[8] - L[2]) / w;
	//float qz = (L[1] - L[4]) / w;
	// http://forum.unity3d.com/threads/how-to-assign-matrix4x4-to-transform.121966/
	float tr = L[0] + L[5] + L[10];
	float w, qw, qx, qy, qz;
	if (tr > 0.0f)			// General case
	{
		w = sqrt(1.0f + tr) * 2.0f;
		qw = 0.25f * w;
		qx = (L[6] - L[9]) / w;
		qy = (L[8] - L[2]) / w;
		qz = (L[1] - L[4]) / w;
	}
	// Cases with w = 0
	else if ((L[0] > L[5]) && (L[0] > L[10]))
	{
		w = sqrt(1.0f + L[0] - L[5] - L[10]) * 2.0f;
		qw = (L[6] - L[9]) / w;
		qx = 0.25f * w;
		qy = -(L[1] + L[4]) / w;
		qz = -(L[8] + L[2]) / w;
	}
	else if (L[5] > L[10])
	{
		w = sqrt(1.0f + L[5] - L[0] - L[10]) * 2.0f;
		qw = (L[8] - L[2]) / w;
		qx = -(L[1] + L[4]) / w;
		qy = 0.25f * w;
		qz = -(L[6] + L[9]) / w;
	}
	else
	{
		w = sqrt(1.0f + L[10] - L[0] - L[5]) * 2.0f;
		qw = (L[1] - L[4]) / w;
		qx = -(L[8] + L[2]) / w;
		qy = -(L[6] + L[9]) / w;
		qz = 0.25f * w;
	}

	Common::CQuaternion unityQuaternion = Common::CQuaternion(qw, qx, qy, qz);
	listenerTransform.SetOrientation(unityQuaternion.Inverse());
	return listenerTransform;
}

Common::CTransform ComputeSourceTransformFromMatrix(float* sourceMatrix, float scale)
{
	// Orientation does not matters for audio sources
	Common::CTransform sourceTransform;
	sourceTransform.SetPosition(Common::CVector3(sourceMatrix[12] * scale, sourceMatrix[13] * scale, sourceMatrix[14] * scale));
	return sourceTransform;
}

//==============================================================================
static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
DistanceAttenuationCallback (UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut)
{
	*attenuationOut = attenuationIn;
	return UNITY_AUDIODSP_OK;
}

//==============================================================================
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback (UnityAudioEffectState* state)
{
    BRTLibraryWrapper::initOrReplace (state->samplerate, state->dspbuffersize);
    
    auto* brtInstance = BRTLibraryWrapper::instance();
    auto instanceId = brtInstance->getNextSoundSourceId();
    
    WriteLog ("BRT: Created Spatializer Source " + std::to_string (instanceId));

    state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;

	EffectData* effectdata = new EffectData;
    effectdata->inMonoBuffer.resize (state->dspbuffersize);
    effectdata->sourceID = instanceId;
    state->effectdata = effectdata;

	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback (UnityAudioEffectState* state)
{
	if (EffectData* data = state->GetEffectData<EffectData>())
    {
        if (auto* brtInstance = BRTLibraryWrapper::instance())
        {
            if (auto soundSource = brtInstance->brtManager.GetSoundSource (std::to_string (data->sourceID)))
            {
                const ScopedManagerSetup sm (brtInstance->brtManager);
                
                auto sourceId = soundSource->GetID();
                
                if (brtInstance->brtManager.RemoveSoundSource (sourceId))
                    brtInstance->releaseSoundSourceId (std::stoi (sourceId));
                else
                    WriteLog ("BRT: Error removing sound source: " + sourceId);
            }
        }
        
        delete data;
    }
	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback (UnityAudioEffectState* state, int index, float value)
{
    return UNITY_AUDIODSP_ERR_UNSUPPORTED;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback (UnityAudioEffectState* state, int index, float* value, char* valuestr)
{
    EffectData* data = state->GetEffectData<EffectData>();
    
	if (valuestr != NULL)
	{
		valuestr[0] = '\0';
	}
    
    WriteLog ("BRT: Getting float parameter " + std::to_string (index));

	if (value != NULL)
	{
		switch (static_cast<Parameter> (index))
		{
            case InstanceId:
                WriteLog ("BRT: Returning sourceID " + std::to_string (data->sourceID));
                *value = (float) data->sourceID;
                break;
            default:
                return UNITY_AUDIODSP_ERR_UNSUPPORTED;
		}
	}
	return UNITY_AUDIODSP_OK;
}

int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
{
	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
ProcessCallback (UnityAudioEffectState* state, float* inbuffer, float* outbuffer,
                 unsigned int length, int inchannels, int outchannels)
{
    EffectData* data = state->GetEffectData<EffectData>();
    
    auto* brtInstance = BRTLibraryWrapper::instance();

    if (! brtInstance || ! brtInstance->isCompatible (state->samplerate, state->dspbuffersize))
        return UNITY_AUDIODSP_ERR_UNSUPPORTED;
    
	// Set source and listener transform
    auto soundSource = brtInstance->brtManager.GetSoundSource (std::to_string (data->sourceID));
    
    if (soundSource)
        soundSource->SetSourceTransform (ComputeSourceTransformFromMatrix (state->spatializerdata->sourcematrix,
                                                                           data->scaleFactor));
    
    if (brtInstance->listener)
        brtInstance->listener->SetListenerTransform (ComputeListenerTransformFromMatrix (state->spatializerdata->listenermatrix,
                                                                                         data->scaleFactor));
	// Transform input buffer
	for (size_t i = 0; i < length; i++)
		data->inMonoBuffer[i] = (inbuffer[i * 2] + inbuffer[i * 2 + 1]) / 2.0f;	// Average of left and right channels

    if (soundSource)
        soundSource->SetBuffer (data->inMonoBuffer);

    for (size_t i = 0; i < (size_t) length * std::max (inchannels, outchannels); ++i)
    {
        outbuffer[i] = inbuffer[i];
    }

	return UNITY_AUDIODSP_OK;
}
}
