using UnityEditor;
using UnityEngine;

namespace BRT.Editor
{
    public class BRTSpatializerWindow : EditorWindow
    {
        private BRTConfiguration[] _presets;
        private int _selectedIndex = -1;

        // ------------------------------------------------------------
        // Open Window
        // ------------------------------------------------------------

        [MenuItem("BRT/Spatializer")]
        public static void ShowWindow()
        {
            GetWindow<BRTSpatializerWindow>("BRT Spatializer");
        }

        // ------------------------------------------------------------
        // Lifecycle
        // ------------------------------------------------------------

        private void OnEnable()
        {
            LoadPresets();
            SyncSelection();
        }

        private void OnFocus()
        {
            SyncSelection();
            Repaint();
        }

        // ------------------------------------------------------------
        // UI
        // ------------------------------------------------------------

        private void OnGUI()
        {
            EditorGUILayout.Space(5);
            EditorGUILayout.LabelField("BRT Spatializer", EditorStyles.boldLabel);

            if (_presets == null || _presets.Length == 0)
            {
                EditorGUILayout.HelpBox("No BRTConfiguration presets found in any Resources folder.", MessageType.Warning);

                if (GUILayout.Button("Refresh"))
                    LoadPresets();

                return;
            }

            DrawPresetDropdown();

            EditorGUILayout.Space(10);

            DrawActiveConfigInfo();

            EditorGUILayout.Space(10);

            if (GUILayout.Button("Refresh"))
            {
                LoadPresets();
                SyncSelection();
            }
        }

        private void DrawPresetDropdown()
        {
            string[] names = System.Array.ConvertAll(_presets, p => p.name);

            EditorGUI.BeginChangeCheck();

            int newIndex = EditorGUILayout.Popup("Preset", _selectedIndex, names);

            if (EditorGUI.EndChangeCheck())
            {
                if (newIndex >= 0 && newIndex < _presets.Length)
                {
                    _selectedIndex = newIndex;
                    BRTSystem.SetConfig(_presets[_selectedIndex]);
                }
            }
        }

        private void DrawActiveConfigInfo()
        {
            EditorGUILayout.LabelField("Active Config", EditorStyles.boldLabel);

            var active = BRTSystem.ActiveConfig;

            if (active == null)
            {
                EditorGUILayout.LabelField("None");
                return;
            }

            EditorGUILayout.LabelField(active.name);
        }

        // ------------------------------------------------------------
        // Internal
        // ------------------------------------------------------------

        private void LoadPresets()
        {
            _presets = Resources.LoadAll<BRTConfiguration>("");
        }

        private void SyncSelection()
        {
            var active = BRTSystem.ActiveConfig;

            if (_presets == null || _presets.Length == 0)
            {
                _selectedIndex = -1;
                return;
            }

            _selectedIndex = -1;

            if (active == null)
                return;

            for (int i = 0; i < _presets.Length; i++)
            {
                if (_presets[i] == active)
                {
                    _selectedIndex = i;
                    return;
                }
            }
        }
    }
}