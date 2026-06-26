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
        [MarshalAs(UnmanagedType.I1)] 
        public bool enabled;
        public float gain;
        [MarshalAs(UnmanagedType.I1)] 
        public bool directPathEnabled;
        [MarshalAs(UnmanagedType.I1)] 
        public bool reverbPathEnabled;
        [MarshalAs(UnmanagedType.I1)] 
        public bool propagationDelayEnabled;
        [MarshalAs(UnmanagedType.I1)] 
        public bool distanceAttenuationEnabled;
        public float distanceAttenuationFactor;
        public float roomLength;
        public float roomWidth;
        public float roomHeight; 
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 54)] 
        public float[] wallAbsorptionCoefficients;
        
        public static EnvironmentModelParameters Default()
        {
            return new EnvironmentModelParameters
            {
                type = EnvironmentModelType.FreeFieldEnvironment,
                enabled = true,
                gain = 1.0f,
                directPathEnabled = true,
                reverbPathEnabled = true,
                propagationDelayEnabled = true,
                distanceAttenuationEnabled = true,
                distanceAttenuationFactor = -6.0206f,
                roomLength = 1.0f,
                roomWidth = 1.0f,
                roomHeight = 1.0f,
                wallAbsorptionCoefficients = new float[54],
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