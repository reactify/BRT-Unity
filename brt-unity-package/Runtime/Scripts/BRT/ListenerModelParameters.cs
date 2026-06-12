using UnityEngine;
using System.Runtime.InteropServices;

namespace BRT
{
    [System.Serializable]
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ListenerModelParameters
    {
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