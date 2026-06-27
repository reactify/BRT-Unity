/**
 * BRT-Unity: Core
**/

#include "BRTLibraryWrapper.h"
#include "AudioPluginUtil.h"
#include "AppUtils.h"
#include "Logging.h"

//==============================================================================
namespace BRTManager
{
    using namespace BRTUnity;

    static std::atomic<int> g_instanceCount {0};

	enum Parameter
	{
		P_MIX = 0,
        P_GAIN = 1,
		P_NUM = 2,
	};

	struct EffectData
	{
		std::array<float, P_NUM> params;
        float scaleFactor = 1.0f;
	};

    //==========================================================================
	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback (UnityAudioEffectState* state)
    {
        auto effectdata = new EffectData;
        memset (effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;
        
        int count = ++g_instanceCount;

       if (count > 1)
           BRT_Log (1, "WARNING: Multiple BRT MANAGER instances created " + std::to_string (count));
       else
           BRT_Log (0, "CREATE BRT MANAGER");
        
        BRTLibraryWrapper::initOrReplace (state->samplerate, state->dspbuffersize);

		return UNITY_AUDIODSP_OK;
	}

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback (UnityAudioEffectState* state)
    {
        int count = --g_instanceCount;
        
        BRT_Log (0, "DELETE BRT MANAGER (remaining: " + std::to_string (count) + ")");
        
        BRTLibraryWrapper::destroy();
        
        if (EffectData* data = state->GetEffectData<EffectData>())
            delete data;
        
        return UNITY_AUDIODSP_OK;
    }

	int InternalRegisterEffectDefinition (UnityAudioEffectDefinition& definition)
	{
		definition.paramdefs = new UnityAudioParameterDefinition[P_NUM];
        AudioPluginUtil::RegisterParameter (definition, "Mix", "", 0.0f, 1.0f, 0.5f,
                           1.0f, 1.0f, P_MIX, "Ratio of spatializer output to dry input audio in output mix");
        AudioPluginUtil::RegisterParameter (definition, "Gain", "", 0.0f, 1.0f, 1.0f,
                           1.0f, 1.0f, P_GAIN, "Gain of dry audio in output mix");
		return P_NUM;
	}

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        data->params[index] = value;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char *valuestr)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        if (value != NULL)
            *value = data->params[index];
        if (valuestr != NULL)
            valuestr[0] = 0;
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
        if (g_instanceCount.load() > 1)
        {
            BRT_Log (1, "WARNING: Multiple BRT MANAGER instances active during processing");
        }
        
        auto brt = BRTLibraryWrapper::instance();

        if (inchannels != 2 || outchannels != 2 ||
          ! brt || ! brt->isCompatible (state->samplerate, state->dspbuffersize))
        {
            memcpy (outbuffer, inbuffer, length * outchannels * sizeof (float));
            return UNITY_AUDIODSP_OK;
        }
        
        brt->process (inbuffer, outbuffer, length, inchannels, outchannels);
        
        EffectData* data = state->GetEffectData<EffectData>();
        
        float wet = data->params[P_MIX];
        float dry  = 1.0f - wet;
        float gain = data->params[P_GAIN];
        
        for (unsigned int i = 0; i < length * outchannels; ++i)
        {
            float in  = inbuffer[i];
            float wetSample = outbuffer[i];

            outbuffer[i] = (dry * in) + (wet * wetSample);
            outbuffer[i] *= gain;
        }

		return UNITY_AUDIODSP_OK;
	}
}
