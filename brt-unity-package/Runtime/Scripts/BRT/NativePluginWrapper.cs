namespace BRT
{
    using UnityEngine;

    public static class NativePluginWrapper
    {
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
    }
}