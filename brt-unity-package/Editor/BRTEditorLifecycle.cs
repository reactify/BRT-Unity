#if UNITY_EDITOR
using UnityEditor;

namespace BRT.Editor
{
    [InitializeOnLoad]
    internal static class BRTEditorLifecycle
    {
        static BRTEditorLifecycle()
        {
            EditorApplication.playModeStateChanged += OnPlayModeChanged;
        }

        private static void OnPlayModeChanged(PlayModeStateChange state)
        {
            switch (state)
            {
                case PlayModeStateChange.EnteredPlayMode:
                    BRTSystem.Initialize();
                    break;

                case PlayModeStateChange.ExitingPlayMode:
                    BRTSystem.Shutdown();
                    BRTSystem.ResetStatics();
                    break;
            }
        }
    }
}
#endif