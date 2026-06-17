using UnityEngine;
using System.Runtime.InteropServices;

namespace BRT
{
    public enum ListenerModelType : int
    {
        DirectHRTFConvolutionModel = 0,
        AmbisonicVirtualLoudspeakersModel = 1,
        DirectBRIRConvolutionModel = 2,
        AmbisonicReverberantVirtualLoudspeakersModel = 3
    }
    
    [System.Serializable]
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ListenerModelParameters
    {
        public ListenerModelType type;
        [MarshalAs(UnmanagedType.I1)] public bool enabled;
        [MarshalAs(UnmanagedType.I1)] public bool spatializationEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool interpolationEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool itdSimulationEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool nearFieldEffectEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool parallaxCorrectionEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool distanceAttenuationEnabled;
        
        public static ListenerModelParameters Default()
        {
            return new ListenerModelParameters
            {
                type = ListenerModelType.DirectBRIRConvolutionModel,
                enabled = true,
                spatializationEnabled = true,
                interpolationEnabled = true,
                itdSimulationEnabled = true,
                nearFieldEffectEnabled = true,
                parallaxCorrectionEnabled = true,
                distanceAttenuationEnabled = true
            };
        }
    }
}