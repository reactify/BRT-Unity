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

        public static void SetHrtfResource(int instanceId, string sofaFile)
        {
            Debug.Log($"[Plugin] SetHrtfResource: {instanceId} {sofaFile}");
        }

        public static void SetBrirResource(int instanceId, string sofaFile)
        {
            Debug.Log($"[Plugin] SetBrirResource: {instanceId} {sofaFile}");
        }
    }
}