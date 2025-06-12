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

        // Declare the delegate type that matches the native callback signature
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ErrorCallback([MarshalAs(UnmanagedType.LPStr)] string message);

        // Import the native method that registers the callback
        [DllImport(DLL_NAME)]
        private static extern void SetErrorCallback(ErrorCallback callback);

        // Keep a reference to the delegate so it doesn't get garbage collected
        private static readonly ErrorCallback callbackDelegate = OnError;

        // Annotated callback method (must be static, and have the MonoPInvokeCallback attribute)
        [MonoPInvokeCallback(typeof(ErrorCallback))]
        private static void OnError([MarshalAs(UnmanagedType.LPStr)] string message)
        {
            Debug.LogError($"[BRT NATIVE] Error: {message}");
        }

        static NativePluginWrapper()
        {
            SetErrorCallback(callbackDelegate);
        }

        [DllImport(DLL_NAME)]
        public static extern void BRTSpatializerResetIfNeeded(int sampleRate, int dspBufferSize);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerCreateListener(string listenerId);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerCreateListenerModel(int type, string modelName);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerConnectListenerModel(string listenerId, string listenerModelId);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerLoadHRTF(string filePath);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerLoadNearFieldCompensationFilter(string nfcFilterFile);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerLoadBRIR(string filePath);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerCreateSoundSource (string soundSourceID);

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerConnectSoundSource (string soundSourceID, string listenerModelID);

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