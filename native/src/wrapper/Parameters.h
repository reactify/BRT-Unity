
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

constexpr int ENVIRONMENT_MODEL_SHOEBOX_WALL_COUNT = 6;
constexpr int ENVIRONMENT_MODEL_WALL_ABSORPTION_BAND_COUNT = 9;
constexpr int ENVIRONMENT_MODEL_WALL_ABSORPTION_COEFFICIENT_COUNT =
    ENVIRONMENT_MODEL_SHOEBOX_WALL_COUNT *
    ENVIRONMENT_MODEL_WALL_ABSORPTION_BAND_COUNT;

static_assert (ENVIRONMENT_MODEL_WALL_ABSORPTION_COEFFICIENT_COUNT == 54,
               "EnvironmentModelParameters expects 6 walls x 9 absorption coefficients.");

#pragma pack(push, 1)
struct EnvironmentModelParameters
{
    EnvironmentModelType type;
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

    float wallAbsorptionCoefficients[ENVIRONMENT_MODEL_WALL_ABSORPTION_COEFFICIENT_COUNT];
};
#pragma pack(pop)

} // namespace BRTUnity
