using UnityEngine;
using System.Runtime.InteropServices;

namespace BRT
{
    public enum EnvironmentModelType : int
    {
        FreeFieldEnvironment = 0,
        SDNEnvironment = 1,
    }
    
    [System.Serializable]
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct EnvironmentModelParameters
    {
        public EnvironmentModelType type;
        [MarshalAs(UnmanagedType.I1)] public bool enabled;
        [MarshalAs(UnmanagedType.I1)] public bool directPathEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool reverbPathEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool distanceAttenuationEnabled;
        [MarshalAs(UnmanagedType.I1)] public bool propagationDelayEnabled;
        
        public static EnvironmentModelParameters Default()
        {
            return new EnvironmentModelParameters
            {
                type = EnvironmentModelType.FreeFieldEnvironment,
                enabled = true,
                directPathEnabled = true,
                reverbPathEnabled = true,
                distanceAttenuationEnabled = true,
                propagationDelayEnabled = true,
            };
        }
    }
    
    [System.Serializable]
    public class EnvironmentModel
    {
        public string modelID;
        public string listenerModelID;
        public EnvironmentModelParameters parameters;

        public EnvironmentModel()
        {
            parameters = EnvironmentModelParameters.Default();
        }
    }
}