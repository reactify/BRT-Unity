using System;
using System.Runtime.InteropServices;

namespace BRT.Log.Internal
{
    internal static class LoggerInterop
    {
#if UNITY_IOS && !UNITY_EDITOR
        private const string DLL_NAME = "__Internal";
#else
        private const string DLL_NAME = "AudioPluginBRTUnity";
#endif

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void LogCallback(int level, IntPtr message);

        [DllImport(DLL_NAME)]
        internal static extern void SetLogCallback(LogCallback callback);
    }
}