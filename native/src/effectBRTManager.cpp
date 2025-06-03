/**
 * BRT-Unity: Core
**/

#include "BRTLibraryWrapper.h"
#include "AudioPluginUtil.h"
#include "AppUtils.h"

//==============================================================================
namespace BRTManager
{
    using namespace BRTUnity;

	enum Parameter
	{
		Wetness = 0,
		NumParameters = 1
	};

	struct EffectData
	{
		std::array<float, NumParameters> parameters;
	};

    inline void WriteLog (std::string logText)
    {
        std::cerr << logText << std::endl;
    }

    //==========================================================================
	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback (UnityAudioEffectState* state)
    {
        auto effectdata = new EffectData;
        effectdata->parameters = {
            0.5f, // wetness
        };
        state->effectdata = effectdata;
        
        WriteLog("BRT: CREATE MANAGER");
        
        BRTLibraryWrapper::initOrReplace (state->samplerate, state->dspbuffersize);

		return UNITY_AUDIODSP_OK;
	}

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback (UnityAudioEffectState* state)
    {
        WriteLog ("BRT: DELETE MANAGER");
        
        BRTLibraryWrapper::destroy();
        
        if (EffectData* data = state->GetEffectData<EffectData>())
            delete data;
        
        return UNITY_AUDIODSP_OK;
    }

	int InternalRegisterEffectDefinition (UnityAudioEffectDefinition& definition)
	{
		definition.paramdefs = new UnityAudioParameterDefinition[NumParameters];
		RegisterParameter (definition, "Wetness", "", 0.0f, 1.0f, 0.5f,
                           1.0f, 1.0f, Wetness, "Ratio of reverb to dry audio in output mix");
		return NumParameters;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
    SetFloatParameterCallback (UnityAudioEffectState* state, int index, float value)
	{
		EffectData* data = state->GetEffectData<EffectData>();
		
        if (index < 0 || index >= NumParameters || data == nullptr)
		{
			return UNITY_AUDIODSP_ERR_UNSUPPORTED;
		}
		
        return UNITY_AUDIODSP_OK;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
    GetFloatParameterCallback (UnityAudioEffectState* state, int index, float* value, char *valuestr)
	{
		EffectData* data = state->GetEffectData<EffectData>();
        
		if (index < 0 || index >= NumParameters || data == nullptr)
		{
			return UNITY_AUDIODSP_ERR_UNSUPPORTED;
		}
		
		*value = data->parameters[index];
		
        return UNITY_AUDIODSP_OK;
	}

	int UNITY_AUDIODSP_CALLBACK
    GetFloatBufferCallback (UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
	{
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
    ProcessCallback (UnityAudioEffectState* state, float* inbuffer, float* outbuffer,
                     unsigned int length, int inchannels, int outchannels)
	{
		if (inchannels != 2 || outchannels != 2)
		{
            WriteLog ("BRT: ERROR: Incorrect channel count in BRTManager plugin");
			return UNITY_AUDIODSP_ERR_UNSUPPORTED;
		}
        
        auto* brtInstance = BRTLibraryWrapper::instance();

        if (! brtInstance || ! brtInstance->isCompatible (state->samplerate, state->dspbuffersize))
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;

        brtInstance->process (inbuffer, outbuffer, length, inchannels, outchannels);

		return UNITY_AUDIODSP_OK;
	}
}
