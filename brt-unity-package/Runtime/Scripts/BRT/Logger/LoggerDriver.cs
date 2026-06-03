using UnityEngine;

namespace BRT
{
    internal sealed class LoggerDriver : MonoBehaviour
    {
        private void Update()
        {
            Logger.FlushNative();
        }

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        private static void Init()
        {
            Logger.Initialize();

            var go = new GameObject("BRT.Logger");
            go.hideFlags = HideFlags.HideAndDontSave;
            Object.DontDestroyOnLoad(go);
            go.AddComponent<LoggerDriver>();
        }
    }
}