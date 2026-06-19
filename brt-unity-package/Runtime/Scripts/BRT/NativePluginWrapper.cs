using UnityEngine;
using System.Runtime.InteropServices;
using AOT;

namespace BRT
{
    public static class NativePluginWrapper
    {
#if UNITY_IOS && !UNITY_EDITOR
        private const string DLL_NAME = "__Internal";
#else
        private const string DLL_NAME = "AudioPluginBRTUnity";
#endif

        [DllImport(DLL_NAME)]
        public static extern void BRTSpatializerResetIfNeeded(int sampleRate, int dspBufferSize);

        [DllImport(DLL_NAME)]
        public static extern void BRTSpatializerDestroy();

        [DllImport(DLL_NAME)]
        public static extern bool BRTCreateListener(string listenerId);
        
        
        [DllImport(DLL_NAME)]
        public static extern bool BRTRemoveListener(string listenerId);

        [DllImport(DLL_NAME)]
        public static extern bool BRTCreateListenerModel(int type, string modelName);

        [DllImport(DLL_NAME)]
        public static extern bool BRTConnectListenerModel(string listenerId, string listenerModelId);

        [DllImport(DLL_NAME)]
        public static extern void BRTClearGraph();
        
        [DllImport(DLL_NAME)]
        public static extern void BRTSetListenerModelParameters(string listenerModelId, ref ListenerModelParameters parameters);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerLoadHRTF(string filePath);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerLoadNearFieldCompensationFilter(string nfcFilterFile);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerLoadBRIR(string filePath);
        
        [DllImport(DLL_NAME)]
        public static extern bool BRTLoadSourceDirectivityTF (string soundSourceID, string directivityFile);

        [DllImport(DLL_NAME)]
        public static extern void BRTSetSourceDirectivityEnabled (string soundSourceID, bool enabled);

        [DllImport(DLL_NAME)]
        public static extern bool BRTCreateSoundSource (string soundSourceID);

        [DllImport(DLL_NAME)]
        public static extern bool BRTConnectSoundSource (string soundSourceID, string listenerModelID);

        [DllImport(DLL_NAME)]
        public static extern void BRTReconnectAllSoundSources();

        [DllImport(DLL_NAME)]
        public static extern bool BRTManagerSetBypassed (bool bypass);

        [DllImport(DLL_NAME)]
        public static extern bool BRTManagerSetSpatializationEnabled (bool bypass);

        [DllImport(DLL_NAME)]
        public static extern bool BRTManagerSetInterpolationEnabled (bool bypass);

        [DllImport(DLL_NAME)]
        public static extern bool BRTManagerSetITDSimulationEnabled (bool bypass);

        [DllImport(DLL_NAME)]
        public static extern bool BRTManagerSetNearFieldEffectEnabled (bool bypass);

        [DllImport(DLL_NAME)]
        public static extern bool BRTManagerSetParallaxCorrectionEnabled (bool bypass);

        [DllImport(DLL_NAME)]
        public static extern  bool BRTManagerSetDistanceAttenuationEnabled (bool bypass);
    }
}