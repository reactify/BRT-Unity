
#include "SpatialiserCore.h"
#include "AppUtils.h"

namespace BRTSpatialiserCore
{
    inline void WriteLog (std::string logText)
    {
        std::cerr << logText << std::endl;
    }

	extern "C" UNITY_AUDIODSP_EXPORT_API
    bool BRTSpatialiserResetIfNeeded (int sampleRate, int dspBufferSize)
	{
		std::lock_guard<std::mutex> lock (SpatialiserCore::mutex());

		return SpatialiserCore::resetInstanceIfNecessary(sampleRate, dspBufferSize);
	}

	extern "C" UNITY_AUDIODSP_EXPORT_API
    bool BRTSpatialiserLoadBinary (BinaryRole role, const char* path, int currentSampleRate, int dspBufferSize)
	{
		std::lock_guard<std::mutex> lock(SpatialiserCore::mutex());

		SpatialiserCore* instance = SpatialiserCore::instance(currentSampleRate, dspBufferSize);
		if (instance == nullptr)
		{
			WriteLog ("Error: setup3DTISpatializer called with incorrect sample rate or buffer size.");
			return false;
		}
		return instance->loadBinary(role, path);
	}

	extern "C" UNITY_AUDIODSP_EXPORT_API
    bool BRTSpatialiserSetFloat (int parameter, float value)
	{
		std::lock_guard<std::mutex> lock(SpatialiserCore::mutex());

		SpatialiserCore* spatializer = SpatialiserCore::instance();
		if (spatializer == nullptr)
		{
			return false;
		}
		return spatializer->SetFloat(parameter, value);
	}

	extern "C" UNITY_AUDIODSP_EXPORT_API
    bool BRTSpatialiserGetFloat (int parameter, float* value)
	{
		assert(value != nullptr);

		std::lock_guard<std::mutex> lock(SpatialiserCore::mutex());

		SpatialiserCore* spatializer = SpatialiserCore::instance();

		return spatializer->GetFloat(parameter, value);
	}

    const std::string LISTENER_ID = "listener1";
    const std::string LISTENER_HRTF_MODEL_ID = "listenerHRTF";
    const std::string LISTENER_BRIR_MODEL_ID = "listenerAmbisonicBRIR";
    const std::string SOUND_SOURCE_ID = "soundSource";

	SpatialiserCore::SpatialiserCore (UInt32 sampleRate, UInt32 bufferSize)
      : scaleFactor (1.0f),
        isLimiterEnabled (true),
        enableReverbProcessing (false)
	{
		perSourceInitialValues[EnableHRTFInterpolation] = 1.0f;
		perSourceInitialValues[EnableFarDistanceLPF] = 1.0f;
		perSourceInitialValues[EnableDistanceAttenuationAnechoic] = 1.0f;
		perSourceInitialValues[EnableNearFieldEffect] = 1.0f;
		perSourceInitialValues[SpatializationMode] = 0.0f;

		const float LimiterThreshold = -30.0f;
		const float LimiterAttack = 500.0f;
		const float LimiterRelease = 500.0f;
		const float LimiterRatio = 6;
		// limiter.Setup(sampleRate, LimiterRatio, LimiterThreshold, LimiterAttack, LimiterRelease);
        
        globalParameters.SetSampleRate (sampleRate);
        globalParameters.SetBufferSize (bufferSize);
        
        const BRTHelpers::ScopedManagerSetup sm (brtManager);
        
        listener = brtManager.CreateListener<BRTBase::CListener> (LISTENER_ID);
        
        listenerHRTFModel = brtManager.CreateListenerModel<BRTListenerModel::CListenerHRTFModel> (LISTENER_HRTF_MODEL_ID);
        if (listenerHRTFModel == nullptr)
            WriteLog ("BRT: Error creating listener model");
    
        if (! listener->ConnectListenerModel (LISTENER_HRTF_MODEL_ID))
            WriteLog ("BRT: Error connecting listener model");

        listenerBRIRModel = brtManager.CreateListenerModel<BRTListenerModel::CListenerAmbisonicEnvironmentBRIRModel> (LISTENER_BRIR_MODEL_ID);
        if (listenerBRIRModel == nullptr)
            WriteLog ("BRT: Error creating listener model");
       
        if (! listener->ConnectListenerModel (LISTENER_BRIR_MODEL_ID))
            WriteLog ("BRT: Error connecting listener model");
	}


	SpatialiserCore::~SpatialiserCore()
	{
		assert(instancePtr() == this);
		instancePtr() = nullptr;
	}

	bool SpatialiserCore::loadBinary (BinaryRole role, std::string path)
	{
		const std::string sofaExtension = ".sofa";
		const bool hasSofaExtension = path.size() >= sofaExtension.size() && path.substr(path.size() - sofaExtension.size()) == sofaExtension;

        WriteLog ("BRT: Loading binary of role " + std::to_string (role) + " : " + path);
        
		switch (role)
		{
		case HighQualityHRTF:
			if (hasSofaExtension)
			{
				// We assume an ILD file holds the delays, so our SOFA file does not specify delays
				// bool specifiedDelays = false;
				// isBinaryResourceLoaded[HighQualityHRTF] = HRTF::CreateFromSofa(path, listener, specifiedDelays);
                // Load HRTF
                auto hrtf = std::make_shared<BRTServices::CHRTF>();
                bool sofaHRTFLoaded = AppUtils::LoadHRTFSofaFile (path, hrtf);
                // Set one for the listener. We can change it at runtime
                if (sofaHRTFLoaded)
                {
                    WriteLog ("BRT: SOFA HRTF loaded. Setting on listener");
                    isBinaryResourceLoaded[HighQualityHRTF] = listener->SetHRTF (hrtf);
                }
			}
			else // If not sofa file then assume its a 3dti-hrtf file
			{
				// isBinaryResourceLoaded[HighQualityHRTF] = HRTF::CreateFrom3dti(path, listener);
			}
			return isBinaryResourceLoaded[HighQualityHRTF];
		case HighQualityILD:
            {
                // isBinaryResourceLoaded[HighQualityILD] = ILD::CreateFrom3dti_ILDNearFieldEffectTable(path, listener);
                auto sosFilter = std::make_shared<BRTServices::CSOSFilters>();
                bool nearFieldFilterLoaded = AppUtils::LoadNearFieldSOSFilter (path, sosFilter);
                
                if (nearFieldFilterLoaded)
                {
                    WriteLog ("BRT: SOFA NEAR FIELD ILD loaded. Setting on listener");
                    isBinaryResourceLoaded[HighQualityILD] = listener->SetNearFieldCompensationFilters (sosFilter);
                }
                
                return isBinaryResourceLoaded[HighQualityILD];
            }
		case HighPerformanceILD:
			// isBinaryResourceLoaded[HighPerformanceILD] = ILD::CreateFrom3dti_ILDSpatializationTable(path, listener);
			return isBinaryResourceLoaded[HighPerformanceILD];
		case ReverbBRIR:
			if (hasSofaExtension)
			{
                // Load BRIR
                auto brir = std::make_shared<BRTServices::CHRBRIR>();
                bool brirSofaLoaded = AppUtils::LoadBRIRSofaFile (path, brir, 0,0,0,0);
                if (brirSofaLoaded) {
                    WriteLog ("BRT: SOFA BRIR loaded. Setting on listener");
                    isBinaryResourceLoaded[ReverbBRIR] = listener->SetHRBRIR (brir);
                }
			}
			else
			{
                // If not sofa file then assume its a 3dti-hrtf file
				// isBinaryResourceLoaded[ReverbBRIR] = BRIR::CreateFrom3dti(path, environment);
			}
			return isBinaryResourceLoaded[ReverbBRIR];
		default:
			return false;
		}
	}

	bool SpatialiserCore::SetFloat(int parameter, float value)
	{
        WriteLog ("BRT: Setting parameter " + std::to_string (parameter) + " : " + std::to_string (value));
        
		switch (parameter)
		{
		case EnableHRTFInterpolation:
		case EnableFarDistanceLPF:
		case EnableDistanceAttenuationAnechoic:
		case EnableNearFieldEffect:
		case SpatializationMode:
		case EnableReverbSend:
		case EnableDistanceAttenuationReverb:
			perSourceInitialValues[parameter] = value;
			return true;

		case HeadRadius:
		{
			const float min = 0.0f;
			const float max = 1e20f;
            if (auto hrtf = listener->GetHRTF())
                hrtf->SetHeadRadius (std::clamp (value, min, max));
			return true;
		}
		case ScaleFactor:
		{
			const float min = 1e-20f;
			const float max = 1e20f;
			scaleFactor = std::clamp (value, min, max);
			return true;
		}
		case EnableCustomITD:
		{
            if (auto hrtf = listener->GetHRTF())
            {
                if (value == 0.0f)
                {
                    hrtf->DisableWoodworthITD();
                }
                else
                {
                    hrtf->EnableWoodworthITD();
                }
            }
			return true;
		}
		case AnechoicDistanceAttenuation:
		{
			const float min = -30.0f;
			const float max = 0.0f;
            listener->SetDistanceAttenuationFactor (std::clamp (value, min, max));
            return true;
		}
		case ILDAttenuation:
		{
			const float min = 0.0f;
			const float max = 30.0f;
			// listener->SetILDAttenutaion(clamp(value, min, max));
		}
		case SoundSpeed:
		{
			const float min = 10.0f;
			const float max = 1000.0f;
            globalParameters.SetSoundSpeed (std::clamp (value, min, max));
			return true;
		}
		case HearingAidDirectionalityAttenuationLeft:
		{
			const float min = 0.0f;
			const float max = 30.0f;
			// listener->SetDirectionality_dB(Common::T_ear::LEFT, clamp(value, min, max));
		}
		case HearingAidDirectionalityAttenuationRight:
		{
			const float min = 0.0f;
			const float max = 30.0f;
			// listener->SetDirectionality_dB(Common::T_ear::RIGHT, clamp(value, min, max));
			return true;
		}
		case EnableHearingAidDirectionalityLeft:
		{
			if (value == 0.0f)
			{
				// listener->DisableDirectionality(Common::T_ear::LEFT);
			}
			else
			{
				// listener->EnableDirectionality(Common::T_ear::LEFT);
			}
			return true;
		}
		case EnableHearingAidDirectionalityRight:
		{
			if (value == 0.0f)
			{
				// listener->DisableDirectionality(Common::T_ear::RIGHT);
			}
			else
			{
				// listener->EnableDirectionality(Common::T_ear::RIGHT);
			}
		}
		case EnableLimiter:
		{
			isLimiterEnabled = value != 0.0f;
			return true;
		}
		case HRTFResamplingStep:
		{
			const float min = 1.0f;
			const float max = 90.0f;
			// core.SetHRTFResamplingStep((int)clamp(value, min, max));
			return true;
		}
		case EnableReverbProcessing:
			enableReverbProcessing = value != 0.0f;
			return true;
		case ReverbOrder:
            //			static_assert((float)ADIMENSIONAL == 0.0f && (float)BIDIMENSIONAL == 1.0f && (float)THREEDIMENSIONAL == 2.0f, "These values are assumed by this code and the correspond c# enumerations.");
            //			if (value == (float)ADIMENSIONAL)
            //			{
            //				environment->SetReverberationOrder(ADIMENSIONAL);
            //			}
            //			else if (value == (float)BIDIMENSIONAL)
            //			{
            //				environment->SetReverberationOrder(BIDIMENSIONAL);
            //			}
            //			else if (value == (float)THREEDIMENSIONAL)
            //			{
            //				environment->SetReverberationOrder(THREEDIMENSIONAL);
            //			}
            //			else
            //			{
            //				WriteLog("ERROR: Set3DTISpatializerFloat with parameter ReverbOrder only supports values 0.0, 1.0 and 2.0. Value received: " + to_string(value));
            //				return false;
            //			}
		case ReverbDistanceAttenuation:
		{
			const float min = -90.0f;
			const float max = 0.0f;
            listenerBRIRModel->SetDistanceAttenuationFactor (std::clamp (value, min, max));
			return true;
		}
		default:
			return false;
		}
	}

	bool SpatialiserCore::GetFloat (int parameter, float* value)
	{
		assert(value != nullptr);
		if (value == nullptr)
		{
			return false;
		}

		switch (parameter)
		{
		case EnableHRTFInterpolation:
		case EnableFarDistanceLPF:
		case EnableDistanceAttenuationAnechoic:
		case EnableNearFieldEffect:
		case SpatializationMode:
		case EnableReverbSend:
		case EnableDistanceAttenuationReverb:
			*value = perSourceInitialValues[parameter];
			return true;
		case HeadRadius:
            if (auto hrtf = listener->GetHRTF())
                *value = hrtf->GetHeadRadius();
			return true;
		case ScaleFactor:
			*value = scaleFactor;
			return true;
		case EnableCustomITD:
            if (auto hrtf = listener->GetHRTF())
                *value = listener->GetHRTF()->IsWoodworthITDEnabled() ? 1.0f : 0.0f;
			return true;
		case AnechoicDistanceAttenuation:
            *value = listener->GetDistanceAttenuationFactor();
			return true;
		case ILDAttenuation:
			// *value = listener->GetILDAttenutaion();
			// return true;
		case SoundSpeed:
            *value = globalParameters.GetSoundSpeed();
			return true;
		case HearingAidDirectionalityAttenuationLeft:
			// *value = listener->GetAnechoicDirectionalityAttenuation_dB(T_ear::LEFT);
			// return true;
		case HearingAidDirectionalityAttenuationRight:
			// *value = listener->GetAnechoicDirectionalityAttenuation_dB(T_ear::RIGHT);
			// return true;
		case EnableHearingAidDirectionalityLeft:
			// *value = listener->IsDirectionalityEnabled(T_ear::LEFT);
			// return true;
		case EnableHearingAidDirectionalityRight:
			// *value = listener->IsDirectionalityEnabled(T_ear::RIGHT);
			// return true;
		case EnableLimiter:
			*value = isLimiterEnabled ? 1.0f : 0.0f;
			return true;
		case HRTFResamplingStep:
            // *value = (float)HRTFResamplingStep;
			// return true;
		case EnableReverbProcessing:
			*value = (float) enableReverbProcessing;
			return true;
		case ReverbOrder:
			// *value = (float)environment->GetReverberationOrder();
			// return true;
		case ReverbDistanceAttenuation:
            *value = listenerBRIRModel->GetDistanceAttenuationFactor();
			// *value = core.GetMagnitudes().GetReverbDistanceAttenuation();
            return true;
		default:
			*value = std::numeric_limits<float>::quiet_NaN();
			return false;
		}
	}

	SpatialiserCore* SpatialiserCore::instance(UInt32 sampleRate, UInt32 bufferSize)
	{
		SpatialiserCore*& s = instancePtr();
		if (s == nullptr)
		{
			s = new SpatialiserCore (sampleRate, bufferSize);
		}
        if (s->globalParameters.GetSampleRate() != sampleRate ||s->globalParameters.GetBufferSize() != bufferSize)
		{
            throw IncorrectAudioStateException(sampleRate, bufferSize, s->globalParameters.GetSampleRate(), s->globalParameters.GetBufferSize());
		}
		return s;
	}

	SpatialiserCore* SpatialiserCore::instance()
	{
		return instancePtr();
	}

	bool SpatialiserCore::resetInstanceIfNecessary (UInt32 sampleRate, UInt32 bufferSize)
	{
		SpatialiserCore*& s = instancePtr();
        if (s != nullptr && (s->globalParameters.GetSampleRate() != sampleRate || s->globalParameters.GetBufferSize() != bufferSize))
		{
			delete s;
			assert(s == nullptr); // this is done by the destructor
		}
		if (s == nullptr)
		{
			s = new SpatialiserCore(sampleRate, bufferSize);
			return true;
		}
		return false;
	}

	SpatialiserCore*& SpatialiserCore::instancePtr()
	{
		static SpatialiserCore* s = nullptr;
		return s;
	}

}
