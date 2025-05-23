using UnityEngine;
using System.Runtime.InteropServices;

namespace BRT
{
    public static class NativePluginWrapper
    {
#if UNITY_IOS && !UNITY_EDITOR
        private const string DLL_NAME = "__Internal";
#else
        private const string DLL_NAME = "AudioPluginBRTUnity";
#endif

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ErrorCallback([MarshalAs(UnmanagedType.LPStr)] string message);

        [DllImport(DLL_NAME)]
        private static extern void SetErrorCallback(ErrorCallback callback);

        private static readonly ErrorCallback callbackDelegate = OnError;

        static NativePluginWrapper()
        {
            SetErrorCallback(callbackDelegate);
        }

        private static void OnError(string msg)
        {
            UnityEngine.Debug.LogError("[BRT NATIVE] " + msg);
        }

        public static void LoadHRTF(string virtualPath)
        {
            Debug.Log($"[Plugin] LoadHRTF: {virtualPath}");
            // TODO: Native call
        }

        public static void LoadBRIR(string virtualPath)
        {
            Debug.Log($"[Plugin] LoadBRIR: {virtualPath}");
            // TODO: Native call
        }

        public static void SetHrtfResource(int instanceId, string sofaFile)
        {
            Debug.Log($"[Plugin] SetHrtfResource: {instanceId} {sofaFile}");
        }

        public static void SetBrirResource(int instanceId, string sofaFile)
        {
            Debug.Log($"[Plugin] SetBrirResource: {instanceId} {sofaFile}");
        }

        [DllImport(DLL_NAME)]
        public static extern bool BRTSpatializerCreateHRTF(string filePath);
    }
}