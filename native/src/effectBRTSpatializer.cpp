/**
 * BRT-Unity: Binaural Spatializer
**/

#include "SpatialiserCore.h"
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

using namespace BRTSpatialiserCore;

struct EffectData
{
    int sourceID;
    CMonoBuffer<float> inMonoBuffer;
};

enum class SpatialiserParameter : int
{
    instanceId = 0,
};

inline int toIndex (SpatialiserParameter param)
{
    return static_cast<int> (param);
}

int InternalRegisterEffectDefinition (UnityAudioEffectDefinition& definition)
{
	int numparams = FloatParameter::NumSourceParameters;
	definition.paramdefs = new UnityAudioParameterDefinition[numparams];
	//RegisterParameter(definition, "SourceID", "", -1.0f, /*FLT_MAX*/ 1e20f, -1.0f, 1.0f, 1.0f, PARAM_SOURCE_ID, "Source ID for debug");
	RegisterParameter(definition, "HRTFInterp", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableHRTFInterpolation, "HRTF Interpolation method");
	RegisterParameter(definition, "MODfarLPF", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableFarDistanceLPF, "Far distance LPF module enabler");
	RegisterParameter(definition, "MODDistAtt", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableDistanceAttenuationAnechoic, "Enable distance attenuation for anechoic processing");
	RegisterParameter(definition, "MODNFILD", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableNearFieldEffect, "Near distance ILD module enabler");
	RegisterParameter(definition, "SpatMode", "", 0.0f, 2.0f, 0.0f, 1.0f, 1.0f, FloatParameter::SpatializationMode, "Spatialization mode (0=High quality, 1=High performance, 2=None)");
	RegisterParameter(definition, "EnableReverb", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, FloatParameter::EnableReverbSend, "Enable reverb processing");
	RegisterParameter(definition, "RevDistAtt", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, FloatParameter::EnableDistanceAttenuationReverb, "Enable distance attenuation for reverb processing");
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

// Mutex must be locked when calling this
UNITY_AUDIODSP_RESULT SetFloatParameter (SpatialiserCore* spatializer, UnityAudioEffectState* state, int index, float value)
{
    EffectData* data = state->GetEffectData<EffectData>();
    assert (data != nullptr && spatializer != nullptr);

    // Process command sent by C# API
    switch (index)
    {

    case FloatParameter::EnableHRTFInterpolation:
        if (value != 0.0f)
        {
//            data->audioSource->EnableInterpolation();
        }
        else
        {
//            data->audioSource->DisableInterpolation();
        }
//        break;
    case FloatParameter::EnableFarDistanceLPF:
        if (value > 0.0f)
        {
//            data->audioSource->EnableFarDistanceEffect();
            WriteLog ("SET PARAMETER: Far distance LPF is ", "Enabled");
        }
        else
        {
//            data->audioSource->DisableFarDistanceEffect();
        }
        break;
    case FloatParameter::EnableDistanceAttenuationAnechoic:
        if (value > 0.0f)
        {
//            data->audioSource->EnableDistanceAttenuationAnechoic();
//            WriteLog(state, "SET PARAMETER: Distance attenuation is ", "Enabled");
        }
        else
        {
//            data->audioSource->DisableDistanceAttenuationAnechoic();
//            WriteLog(state, "SET PARAMETER: Distance attenuation is ", "Disabled");
        }
    case FloatParameter::EnableNearFieldEffect:
        if (value > 0.0f)
        {
//            data->audioSource->EnableNearFieldEffect();
        }
        else
        {
//            data->audioSource->DisableNearFieldEffect();
        }
//        break;
    case FloatParameter::SpatializationMode:
//        if (value == (float)Binaural::TSpatializationMode::HighQuality)
//        {
//            data->audioSource->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
//        }
//        else if (value == (float)Binaural::TSpatializationMode::HighPerformance)
//        {
//            data->audioSource->SetSpatializationMode(Binaural::TSpatializationMode::HighPerformance);
//        }
//        else if (value == (float)Binaural::TSpatializationMode::NoSpatialization)
//        {
//            data->audioSource->SetSpatializationMode(Binaural::TSpatializationMode::NoSpatialization);
//            WriteLog(state, "SET PARAMETER: No spatialization mode is enabled", "");
//        }
//        else
//        {
//            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
//        }
//        break;
    case FloatParameter::EnableReverbSend:
//        WriteLog ("BRT: Enable Reverb send: " + std::to_string (value));
        if (value != 0.0f)
        {
//            data->audioSource->EnableReverbProcess();
        }
        else
        {
//            data->audioSource->DisableReverbProcess();
        }
//        break;
    case FloatParameter::EnableDistanceAttenuationReverb:
        if (value != 0.0f)
        {
//            data->audioSource->EnableDistanceAttenuationReverb();
        }
        else
        {
//            data->audioSource->DisableDistanceAttenuationReverb();
        }
//        break;
    default:
        WriteLog ("SET PARAMETER: ERROR!!!! Unknown float parameter received from API: ", index);
        return UNITY_AUDIODSP_ERR_UNSUPPORTED;
    }

    return UNITY_AUDIODSP_OK;
}

//==============================================================================
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback (UnityAudioEffectState* state)
{
	if (!IsHostCompatible(state))
	{
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}

	std::lock_guard<std::mutex> lock(SpatialiserCore::mutex());
	SpatialiserCore* spatializer;
	try
	{
		spatializer = SpatialiserCore::instance (state->samplerate, state->dspbuffersize);
	}
	catch (const SpatialiserCore::IncorrectAudioStateException& e)
	{
		WriteLog (std::string("Error: Spatialiser CreateCallback called with incorrect audio state. ") + e.what());
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
    
    WriteLog ("BRT: Created Spatialiser Source");

	// CREATE Instance state and grab parameters

	EffectData* effectdata = new EffectData;
    effectdata->inMonoBuffer.resize (state->dspbuffersize);
    
    // Create sound source
    effectdata->sourceID = spatializer->numSoundSources++;
    
    state->effectdata = effectdata;
    state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;
    
    // Set default parameters
    /*
    if (effectdata->soundSource != nullptr)
    {
        static_assert (std::tuple_size<decltype(SpatialiserCore::perSourceInitialValues)>::value == FloatParameter::NumSourceParameters, "NumSourceParameters should match the size of SpatialiserCore::perSourceInitialValues array.");
        
        // Initialize with defaults
        for (int i = FloatParameter::FirstSourceParameter; i < FloatParameter::NumSourceParameters; ++i)
        {
            float value = 0;
            
            if (spatializer->GetFloat (i, &value))
                SetFloatParameter (spatializer, state, i, value);
        }
    }
     */

	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback (UnityAudioEffectState* state)
{
	if (EffectData* data = state->GetEffectData<EffectData>())
    {
        std::lock_guard<std::mutex> lock (SpatialiserCore::mutex());
        
        SpatialiserCore* spatializer;
        try
        {
            spatializer = SpatialiserCore::instance (state->samplerate, state->dspbuffersize);
        }
        catch (const SpatialiserCore::IncorrectAudioStateException& e)
        {
            WriteLog (std::string ("Error: Spatialiser ReleaseCallback called with incorrect audio state. ") + e.what());
        }
        
        if (auto soundSource = spatializer->brtManager.GetSoundSource (std::to_string (data->sourceID)))
        {
            auto sourceID = soundSource->GetID();
            
            const BRTHelpers::ScopedManagerSetup sm (spatializer->brtManager);
            
            for (std::string listenerModelId : spatializer->brtManager.GetListenerModelIDs())
            {
//                auto listenerModel = spatializer->brtManager.GetListenerModel (listenerModelId);
//                if (! listenerModel->DisconnectSoundSource (sourceID))
//                    WriteLog ("BRT: Error disconnecting sound source from listener model");
            }
            
//            if (! spatializer->brtManager.RemoveSoundSource (sourceID))
//                WriteLog ("BRT: Error removing sound source: " + sourceID);
        }
        
        if (auto listener = spatializer->listener)
        {
            spatializer->listener->RemoveHRTF();
//            
//            for (auto model : spatializer->listenerModels)
//                spatializer->listener->DisconnectListenerModel (model->GetModelID());
        }
        
        delete data;
    }
	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback (UnityAudioEffectState* state, int index, float value)
{
	std::lock_guard<std::mutex> lock (SpatialiserCore::mutex());
	SpatialiserCore* spatializer;
	try
	{
		spatializer = SpatialiserCore::instance(state->samplerate, state->dspbuffersize);
	}
	catch (const SpatialiserCore::IncorrectAudioStateException& e)
	{
		WriteLog(std::string("Error: Reverb ProcessCallback called with incorrect audio state. ") + e.what());
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
	assert(spatializer != nullptr);
	return SetFloatParameter (spatializer, state, index, value);
}

UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback (UnityAudioEffectState* state, int index, float* value, char* valuestr)
{
	std::lock_guard<std::mutex> lock (SpatialiserCore::mutex());
    
	SpatialiserCore* spatializer;
	try
	{
		spatializer = SpatialiserCore::instance (state->samplerate, state->dspbuffersize);
	}
	catch (const SpatialiserCore::IncorrectAudioStateException& e)
	{
		WriteLog (std::string ("Error: Reverb ProcessCallback called with incorrect audio state. ") + e.what());
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}

    EffectData* data = state->GetEffectData<EffectData>();
    
	if (valuestr != NULL)
	{
		valuestr[0] = '\0';
	}

	if (value != NULL)
	{
		switch (static_cast<SpatialiserParameter> (index))
		{
            case SpatialiserParameter::instanceId:
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
	std::lock_guard<std::mutex> lock (SpatialiserCore::mutex());
    
	SpatialiserCore* spatializer;
	try
	{
		spatializer = SpatialiserCore::instance(state->samplerate, state->dspbuffersize);
	}
	catch (const SpatialiserCore::IncorrectAudioStateException& e)
	{
		WriteLog(std::string("Error: Reverb ProcessCallback called with incorrect audio state. ") + e.what());
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}

    EffectData* data = state->GetEffectData<EffectData>();
    
	// Check that I/O formats are right and that the host API supports this feature
	if (inchannels != 2 || outchannels != 2 ||
		!IsHostCompatible(state) || state->spatializerdata == NULL)
	{
        auto sourceIDStr = std::to_string (data->sourceID);
        WriteLog ("PROCESS: ERROR!!!! Wrong number of channels or Host is not compatible:", "", sourceIDStr);
		WriteLog ("         Input channels = ", inchannels, sourceIDStr);
		WriteLog ("         Output channels = ", outchannels, sourceIDStr);
		WriteLog ("         Host compatible = ", IsHostCompatible (state), sourceIDStr);
		WriteLog ("         Spatializer data exists = ", (state->spatializerdata != NULL), sourceIDStr);
		WriteLog ("         Buffer length = ", length, sourceIDStr);
		// Return silence on error.
		std::fill (outbuffer, outbuffer + length * (size_t)outchannels, 0.0f);
		return UNITY_AUDIODSP_OK;
	}

	  // Set source and listener transform
    auto soundSource = spatializer->brtManager.GetSoundSource (std::to_string (data->sourceID));
    if (soundSource)
    {
        soundSource->SetSourceTransform (ComputeSourceTransformFromMatrix (state->spatializerdata->sourcematrix, spatializer->scaleFactor));
    }
    
    if (spatializer->listener)
        spatializer->listener->SetListenerTransform (ComputeListenerTransformFromMatrix (state->spatializerdata->listenermatrix, spatializer->scaleFactor));

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
