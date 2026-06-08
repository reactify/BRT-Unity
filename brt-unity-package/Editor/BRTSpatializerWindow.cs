using UnityEditor;
using UnityEngine;

namespace BRT.Editor
{
    public class BRTSpatializerWindow : EditorWindow
    {
        private BRTConfiguration[] _presets;
        private int _selectedIndex = -1;

        private BRTConfiguration _sourcePreset;

        private UnityEditor.Editor _runtimeEditor;

        // --------------------------------------------------------------------
        // Window
        // --------------------------------------------------------------------

        [MenuItem("BRT/Spatializer")]
        public static void Open()
        {
            GetWindow<BRTSpatializerWindow>("BRT Spatializer");
        }

        // --------------------------------------------------------------------
        // Lifecycle
        // --------------------------------------------------------------------

        private void OnEnable()
        {
            LoadPresets();
            SyncToActiveConfig();
        }

        private void OnDisable()
        {
            if (_runtimeEditor != null)
            {
                DestroyImmediate(_runtimeEditor);
                _runtimeEditor = null;
            }
        }

        private void OnFocus()
        {
            SyncToActiveConfig();
            Repaint();
        }

        // --------------------------------------------------------------------
        // GUI
        // --------------------------------------------------------------------

        private void OnGUI()
        {
            EditorGUILayout.Space(5);
            EditorGUILayout.LabelField("BRT Spatializer", EditorStyles.boldLabel);

            if (_presets == null || _presets.Length == 0)
            {
                EditorGUILayout.HelpBox(
                    "No BRTConfiguration presets found in Resources.",
                    MessageType.Warning);

                if (GUILayout.Button("Refresh"))
                    LoadPresets();

                return;
            }

            DrawPresetSelector();

            EditorGUILayout.Space(10);

            DrawActiveInfo();

            EditorGUILayout.Space(10);

            DrawRuntimeInspector();

            EditorGUILayout.Space(10);

            if (GUILayout.Button("Refresh Presets"))
            {
                LoadPresets();
                SyncToActiveConfig();
            }
        }

        // --------------------------------------------------------------------
        // Preset selection
        // --------------------------------------------------------------------

        private void DrawPresetSelector()
        {
            string[] names = new string[_presets.Length];

            for (int i = 0; i < _presets.Length; i++)
            {
                var name = _presets[i] ? _presets[i].name : "<Missing>";

                if (_presets[i] == _sourcePreset && IsDirty())
                    name += " *";

                names[i] = name;
            }

            EditorGUILayout.BeginHorizontal();

            int newIndex = EditorGUILayout.Popup("Preset", _selectedIndex, names);

            bool reloadClicked = GUILayout.Button("Reload", GUILayout.Width(70));

            if (newIndex != _selectedIndex || reloadClicked)
            {
                _selectedIndex = newIndex;

                if (_selectedIndex >= 0 && _selectedIndex < _presets.Length)
                {
                    _sourcePreset = _presets[_selectedIndex];

                    BRTSystem.SetConfig(_sourcePreset);

                    SyncToActiveConfig();
                }
            }

            EditorGUILayout.EndHorizontal();
        }

        // --------------------------------------------------------------------
        // Active info
        // --------------------------------------------------------------------

        private void DrawActiveInfo()
        {
            EditorGUILayout.LabelField("Active Runtime Config", EditorStyles.boldLabel);

            var active = BRTSystem.ActiveConfig;

            if (active == null)
            {
                EditorGUILayout.LabelField("None");
                return;
            }

            string label = active.name;

            if (_sourcePreset != null && IsDirty())
                label += " (Modified)";

            EditorGUILayout.LabelField(label);
        }

        // --------------------------------------------------------------------
        // Runtime inspector
        // --------------------------------------------------------------------

        private void DrawRuntimeInspector()
        {
            var active = BRTSystem.ActiveConfig;

            if (active == null)
                return;

            if (_runtimeEditor == null || _runtimeEditor.target != active)
            {
                if (_runtimeEditor != null)
                    DestroyImmediate(_runtimeEditor);

                _runtimeEditor = UnityEditor.Editor.CreateEditor(active);
            }

            EditorGUILayout.LabelField("Runtime Configuration", EditorStyles.boldLabel);

            EditorGUI.BeginChangeCheck();

            _runtimeEditor.OnInspectorGUI();

            if (EditorGUI.EndChangeCheck())
            {
                _runtimeEditor.serializedObject.ApplyModifiedProperties();
                
                BRTSystem.ReapplyRuntimeConfig();

                Repaint();
            }
        }

        // --------------------------------------------------------------------
        // Sync
        // --------------------------------------------------------------------

        private void SyncToActiveConfig()
        {
            var active = BRTSystem.ActiveConfig;

            _selectedIndex = -1;

            if (_presets != null && _sourcePreset != null)
            {
                for (int i = 0; i < _presets.Length; i++)
                {
                    if (_presets[i] == _sourcePreset)
                    {
                        _selectedIndex = i;
                        break;
                    }
                }
            }

            if (_runtimeEditor != null)
            {
                DestroyImmediate(_runtimeEditor);
                _runtimeEditor = null;
            }

            if (active != null)
            {
                _runtimeEditor = UnityEditor.Editor.CreateEditor(active);
            }
        }

        // --------------------------------------------------------------------
        // Dirty detection (generic, stable)
        // --------------------------------------------------------------------

        private bool IsDirty()
        {
            var active = BRTSystem.ActiveConfig;

            if (_sourcePreset == null || active == null)
                return false;

            return ComputeHash(_sourcePreset) != ComputeHash(active);
        }

        private int ComputeHash(Object obj)
        {
            var so = new SerializedObject(obj);
            var prop = so.GetIterator();

            int hash = 17;
            bool enterChildren = true;

            while (prop.NextVisible(enterChildren))
            {
                enterChildren = false;

                if (prop.propertyPath == "m_Script")
                    continue;

                hash = hash * 31 + prop.propertyPath.GetHashCode();

                switch (prop.propertyType)
                {
                    case SerializedPropertyType.Integer:
                        hash = hash * 31 + prop.intValue;
                        break;

                    case SerializedPropertyType.Boolean:
                        hash = hash * 31 + (prop.boolValue ? 1 : 0);
                        break;

                    case SerializedPropertyType.Float:
                        hash = hash * 31 + prop.floatValue.GetHashCode();
                        break;

                    case SerializedPropertyType.String:
                        if (prop.stringValue != null)
                            hash = hash * 31 + prop.stringValue.GetHashCode();
                        break;

                    case SerializedPropertyType.Enum:
                        hash = hash * 31 + prop.enumValueIndex;
                        break;

                    case SerializedPropertyType.ObjectReference:
                        hash = hash * 31 + (prop.objectReferenceValue ? prop.objectReferenceValue.GetInstanceID() : 0);
                        break;
                }
            }

            return hash;
        }

        // --------------------------------------------------------------------
        // Loading
        // --------------------------------------------------------------------

        private void LoadPresets()
        {
            _presets = Resources.LoadAll<BRTConfiguration>("");
        }
    }
}