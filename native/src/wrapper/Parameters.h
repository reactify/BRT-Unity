
#pragma once

namespace BRTUnity
{

#pragma pack(push, 1)
struct ListenerModelParameters
{
    bool enabled = true;
    bool spatializationEnabled = true;
    bool interpolationEnabled = true;
    bool itdSimulationEnabled = true;
    bool nearFieldEffectEnabled = true;
    bool parallaxCorrectionEnabled = true;
    bool distanceAttenuationEnabled = true;
};
#pragma pack(pop)

} // namespace BRTUnity
