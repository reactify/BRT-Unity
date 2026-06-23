
#pragma once

#include <cstdint>

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
    int8_t enabled;
    int8_t spatializationEnabled;
    int8_t interpolationEnabled;
    int8_t itdSimulationEnabled;
    int8_t nearFieldEffectEnabled;
    int8_t parallaxCorrectionEnabled;
    int8_t distanceAttenuationEnabled;
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
    int8_t enabled;
    float gain;
    int8_t directPathEnabled;
    int8_t reverbPathEnabled;
    int8_t propagationDelayEnabled;
    int8_t distanceAttenuationEnabled;
    float distanceAttenuationFactor;
    float roomLength;
    float roomWidth;
    float roomHeight;
};
#pragma pack(pop)

} // namespace BRTUnity
