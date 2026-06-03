using UnityEngine;

namespace BRT
{
    internal static class BRTBootstrap
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        private static void Init()
        {
            BRTSystem.Initialize();
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.SubsystemRegistration)]
        private static void Reset()
        {
            // Ensures clean state when domain reload is disabled
            BRTSystem.ResetStatics();
        }

        // Optional: ensure shutdown on quit (player only)
        private class ShutdownHook : MonoBehaviour
        {
            private void OnApplicationQuit()
            {
                BRTSystem.Shutdown();
            }
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        private static void CreateShutdownHook()
        {
            var go = new GameObject("BRTShutdownHook");
            Object.DontDestroyOnLoad(go);
            go.hideFlags = HideFlags.HideAndDontSave;
            go.AddComponent<ShutdownHook>();
        }
    }
}