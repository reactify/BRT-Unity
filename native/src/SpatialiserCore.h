#pragma once

#define NOMINMAX
#include <cfloat>
#include "AudioPluginUtil.h"
#include "AudioPluginInterface.h"
#include "BRTLibrary.h"

namespace BRTHelpers
{
struct ScopedManagerSetup
{
    ScopedManagerSetup (BRTBase::CBRTManager& m)
      : manager (m)       { manager.BeginSetup(); }
    ~ScopedManagerSetup() { manager.EndSetup(); }
    BRTBase::CBRTManager& manager;
};
}

inline bool IsHostCompatible (UnityAudioEffectState* state)
{
    return state->structsize >= sizeof(UnityAudioEffectState)
        && state->hostapiversion >= UNITY_AUDIO_PLUGIN_API_VERSION;
}

inline void WriteLog (std::string logText)
{
    std::cerr << logText << std::endl;
}

namespace BRTSpatialiserCore
{
	// Parameters set outside of the unity Parameter system
	enum FloatParameter : int
	{
		// Values here must be kept in sync with the corresponding enum in c# code.

		// Per-source parameters. We store them in the core so we know what value to initialize the values to on a new source instance.
		EnableHRTFInterpolation = 0, // ### SOURCE ####
		FirstSourceParameter = EnableHRTFInterpolation,
		EnableFarDistanceLPF = 1, // ### SOURCE ####
		EnableDistanceAttenuationAnechoic = 2, // ### SOURCE ####
		EnableNearFieldEffect = 3,// ### SOURCE ####
		SpatializationMode = 4,// ### SOURCE ####
		EnableReverbSend = 5,// ### SOURCE ####
		EnableDistanceAttenuationReverb = 6,// ### SOURCE ####

		NumSourceParameters = 7,

		// Listener parameters
		HeadRadius = 7,
		FirstListenerParameter = HeadRadius,
		ScaleFactor = 8,
		EnableCustomITD = 9,
		AnechoicDistanceAttenuation = 10,
		ILDAttenuation = 11,
		SoundSpeed = 12,
		HearingAidDirectionalityAttenuationLeft = 13,
		HearingAidDirectionalityAttenuationRight = 14,
		EnableHearingAidDirectionalityLeft = 15,
		EnableHearingAidDirectionalityRight = 16,
		EnableLimiter = 17,
		HRTFResamplingStep = 18,
		EnableReverbProcessing = 19,
		ReverbOrder = 20,
		ReverbDistanceAttenuation = 21,

		NumFloatParameters = 22,
	};


	enum BinaryRole
	{
		// Must be kept in sync with c# code
		HighPerformanceILD = 0,
		HighQualityHRTF = 1,
		HighQualityILD = 2,
		ReverbBRIR = 3,
		NumBinaryRoles = 4,
	};

	//==========================================================================
	struct SpatialiserCore
	{
		// Each instance of the reverb effect has an instance of the Core
        Common::CGlobalParameters globalParameters;                             // Class where the global BRT parameters are defined.
        BRTBase::CBRTManager brtManager;                                        // BRT global manager interface
        std::shared_ptr<BRTBase::CListener> listener;                           // Pointer to listener model
        std::shared_ptr<BRTListenerModel::CListenerHRTFModel> listenerHRTFModel;
        std::shared_ptr<BRTListenerModel::CListenerAmbisonicEnvironmentBRIRModel> listenerBRIRModel;
        
		std::array<float, NumSourceParameters> perSourceInitialValues;
		float scaleFactor;
		bool isLimiterEnabled;
		bool enableReverbProcessing;
        UInt32 numSoundSources = 0;
        
		// This mutex must be locked during any use of the spatializer instance, or in the creation/destruction of instances.
		inline static std::mutex& mutex()
		{
			static std::mutex m; 
			return m;
		}

		// Status
		std::array<bool, NumBinaryRoles> isBinaryResourceLoaded = { false, false, false, false };

	protected:
		SpatialiserCore (UInt32 sampleRate, UInt32 bufferSize);

	public:
		~SpatialiserCore();

		bool loadBinary (BinaryRole role, std::string path);

		bool SetFloat (int parameter, float value);
		bool GetFloat (int parameter, float* value);

		class IncorrectAudioStateException : public std::runtime_error
		{
		public:
			IncorrectAudioStateException(UInt32 requestedSampleRate, UInt32 requestedBufferSize, UInt32 existingSampleRate, UInt32 existingBufferSize)
				: std::runtime_error("SpatialiserCore is already running with audio state "+ std::to_string(existingSampleRate)+ ", " + std::to_string(existingBufferSize) + " but instance was now requested with audio state "+ std::to_string(requestedSampleRate)+", "+ std::to_string(requestedBufferSize)+".")
			{}
		};

		// Get an instance to the singleton SpatialiserCore, creating one if necessary or if destroyAnyExistingInstance is true. 
		// If sampleRate or bufferSize doesn't match the existing instance then an IncorrectAudioStateException will be thrown.
		// NB SpatialiserCore::mutex must be locked *before* calling this and remain locked until you are
		// finished with the instance
		// Do not store this value as an instance may be destroyed in the future. Instead re-request it and 
		static SpatialiserCore* instance(UInt32 sampleRate, UInt32 bufferSize);
		// Get an instance to the singleton SpatialiserCore. If none exists currently then returns nullptr.
		// NB SpatialiserCore::mutex must be locked *before* calling this and remain locked until you are
		static SpatialiserCore* instance();
		// Ensures an instance exists with the given sampleRate and bufferSize. If necessary an existing instance is destroyed
		// Returns true if a new instance was created.
		// NB SpatialiserCore::mutex must be locked *before* calling this and remain locked until you are
		static bool resetInstanceIfNecessary(UInt32 sampleRate, UInt32 bufferSize);

	private:
		static SpatialiserCore*& instancePtr();
	};

}
