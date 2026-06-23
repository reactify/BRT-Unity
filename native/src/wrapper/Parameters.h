
#pragma once

namespace BRTUnity
{

enum class ListenerModelType : int
{
    DirectHRTFConvolutionModel = 0,
    AmbisonicVirtualLoudspeakersModel = 1
};

#pragma pack(push, 1)
struct ListenerModelParameters
{
    ListenerModelType type;
    bool enabled = true;
    bool spatializationEnabled = true;
    bool interpolationEnabled = true;
    bool itdSimulationEnabled = true;
    bool nearFieldEffectEnabled = true;
    bool parallaxCorrectionEnabled = true;
    bool distanceAttenuationEnabled = true;
};
#pragma pack(pop)


enum class EnvironmentModelType : int
{
    FreeFieldEnvironment = 0,
    SDNEnvironment = 1
};

#pragma pack(push, 1)
struct EnvironmentModelParameters
{
    ListenerModelType type;
    bool enabled = true;
    bool directPathEnabled = true;
    bool reverbPathEnabled = true;
    bool distanceAttenuationEnabled = true;
    bool propagationDelayEnabled = true;
};
#pragma pack(pop)

} // namespace BRTUnity
