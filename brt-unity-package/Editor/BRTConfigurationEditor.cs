namespace BRT.Editor
{
    using UnityEditor;
    using UnityEngine;
    using System.Collections.Generic;
    using System.Linq;

    [CustomEditor(typeof(BRTConfiguration))]
    public class BRTConfigurationEditor : UnityEditor.Editor
    {
        private SerializedProperty hrtfResources;
        private SerializedProperty brirResources;
        private SerializedProperty directivityResources;
        private SerializedProperty nfcFilterResources;
        private SerializedProperty listenerModels;
        private SerializedProperty listenerEnvironmentModels;

        // --------------------------------------------------------------------
        // CACHES
        // --------------------------------------------------------------------

        private static readonly Dictionary<string, List<string>> _sofaCache = new();

        private static readonly Dictionary<string, string> _popupLabels = new()
        {
            { "HRTFResourceIndex", "HRTF" },
            { "NFCResourceIndex", "NFC Filter" },
            { "BRIRResourceIndex", "BRIR" }
        };

        private void OnEnable()
        {
            hrtfResources = serializedObject.FindProperty("hrtfResources");
            brirResources = serializedObject.FindProperty("brirResources");
            directivityResources = serializedObject.FindProperty("directivityResources");
            nfcFilterResources = serializedObject.FindProperty("nfcFilterResources");
            listenerModels = serializedObject.FindProperty("listenerModels");
            listenerEnvironmentModels = serializedObject.FindProperty("listenerEnvironmentModels");
        }

        public override void OnInspectorGUI()
        {
            serializedObject.Update();

            EditorGUILayout.LabelField("HRTF Resources", EditorStyles.boldLabel);
            DrawResourceList(hrtfResources, BRTConfiguration.HRTFResourceFolder);

            EditorGUILayout.Space(10);

            EditorGUILayout.LabelField("BRIR Resources", EditorStyles.boldLabel);
            DrawResourceList(brirResources, BRTConfiguration.BRIRResourceFolder);

            EditorGUILayout.Space(10);

            EditorGUILayout.LabelField("Directivity Resources", EditorStyles.boldLabel);
            DrawResourceList(directivityResources, BRTConfiguration.DirectivityResourceFolder);

            EditorGUILayout.Space(10);

            EditorGUILayout.LabelField("NFC Filter Resources", EditorStyles.boldLabel);
            DrawResourceList(nfcFilterResources, BRTConfiguration.NFCFilterResourceFolder);

            EditorGUILayout.Space(20);

            DrawListenerModels();

            serializedObject.ApplyModifiedProperties();
        }

        // --------------------------------------------------------------------
        // RESOURCE LIST UI
        // --------------------------------------------------------------------

        private void DrawResourceList(SerializedProperty listProp, string resourcePath)
        {
            var sofaOptions = GetSofaOptions(resourcePath);

            if (sofaOptions.Count == 0)
            {
                EditorGUILayout.HelpBox(
                    $"No SOFA files found in: Resources/{resourcePath}",
                    MessageType.Warning);

                if (GUILayout.Button("Add Entry"))
                    listProp.InsertArrayElementAtIndex(listProp.arraySize);

                return;
            }

            for (int i = 0; i < listProp.arraySize; i++)
            {
                var element = listProp.GetArrayElementAtIndex(i);

                EditorGUILayout.BeginVertical("box");

                var sofaProp = element.FindPropertyRelative("sofaFile");

                int selectedIndex = Mathf.Max(0, sofaOptions.IndexOf(sofaProp.stringValue));
                int newIndex = EditorGUILayout.Popup("SOFA File", selectedIndex, sofaOptions.ToArray());

                sofaProp.stringValue =
                    (newIndex >= 0 && newIndex < sofaOptions.Count)
                        ? sofaOptions[newIndex]
                        : "";

                DrawRemainingFields(element);

                if (GUILayout.Button("Remove"))
                {
                    listProp.DeleteArrayElementAtIndex(i);
                    EditorGUILayout.EndVertical();
                    break;
                }

                EditorGUILayout.EndVertical();
            }

            if (GUILayout.Button("Add Entry"))
                listProp.InsertArrayElementAtIndex(listProp.arraySize);
        }

        private void DrawRemainingFields(SerializedProperty element)
        {
            SerializedProperty copy = element.Copy();
            SerializedProperty end = copy.GetEndProperty();

            copy.NextVisible(true);

            while (!SerializedProperty.EqualContents(copy, end))
            {
                if (copy.name != "sofaFile")
                    EditorGUILayout.PropertyField(copy, true);

                if (!copy.NextVisible(false))
                    break;
            }
        }

        // --------------------------------------------------------------------
        // LISTENER UI
        // --------------------------------------------------------------------

        private void DrawListenerModels()
        {
            if (listenerModels == null || listenerModels.arraySize == 0)
                return;

            EditorGUILayout.LabelField("Listener Model", EditorStyles.boldLabel);
            DrawListenerModel(listenerModels.GetArrayElementAtIndex(0));

            if (listenerEnvironmentModels == null || listenerEnvironmentModels.arraySize == 0)
                return;

            EditorGUILayout.LabelField("Listener Environment Model", EditorStyles.boldLabel);
            DrawListenerModel(listenerEnvironmentModels.GetArrayElementAtIndex(0));
        }

        private void DrawListenerModel(SerializedProperty modelProp)
        {
            EditorGUI.indentLevel++;

            SerializedProperty prop = modelProp.Copy();
            SerializedProperty end = prop.GetEndProperty();

            prop.NextVisible(true);

            while (!SerializedProperty.EqualContents(prop, end))
            {
                if (_popupLabels.TryGetValue(prop.name, out var label))
                {
                    string[] options = GetSofaFileNamesForField(prop.name);

                    int index = Mathf.Clamp(prop.intValue, 0, Mathf.Max(0, options.Length - 1));
                    index = EditorGUILayout.Popup(label, index, options);

                    prop.intValue = index;
                }
                else
                {
                    EditorGUILayout.PropertyField(prop, true);
                }

                if (!prop.NextVisible(false))
                    break;
            }

            EditorGUI.indentLevel--;
        }

        // --------------------------------------------------------------------
        // DATA HELPERS
        // --------------------------------------------------------------------

        private List<string> GetSofaOptions(string resourcePath)
        {
            if (_sofaCache.TryGetValue(resourcePath, out var cached))
                return cached;

            var result = Resources.LoadAll<TextAsset>(resourcePath)
                .Select(x => x.name)
                .ToList();

            _sofaCache[resourcePath] = result;
            return result;
        }

        private string[] GetSofaFileNamesForField(string fieldName)
        {
            SerializedProperty list = fieldName switch
            {
                "HRTFResourceIndex" => hrtfResources,
                "NFCResourceIndex" => nfcFilterResources,
                "BRIRResourceIndex" => brirResources,
                _ => null
            };

            if (list == null)
                return new[] { "<None>" };

            var names = new string[list.arraySize];

            for (int i = 0; i < list.arraySize; i++)
            {
                var element = list.GetArrayElementAtIndex(i);
                var sofa = element.FindPropertyRelative("sofaFile");

                names[i] = !string.IsNullOrEmpty(sofa.stringValue)
                    ? sofa.stringValue
                    : "<Missing>";
            }

            return names;
        }
    }
}