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

        private void OnEnable()
        {
            hrtfResources = serializedObject.FindProperty("hrtfResources");
            brirResources = serializedObject.FindProperty("brirResources");
            directivityResources = serializedObject.FindProperty("directivityResources");
            nfcFilterResources = serializedObject.FindProperty("nfcFilterResources");
            listenerModels = serializedObject.FindProperty("listenerModels");
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

            EditorGUILayout.LabelField("NFCFilter Resources", EditorStyles.boldLabel);
            DrawResourceList(nfcFilterResources, BRTConfiguration.NFCFilterResourceFolder);

            EditorGUILayout.Space(20);

            DrawListenerModelIfExists();

            serializedObject.ApplyModifiedProperties();
        }

        private void DrawResourceList(SerializedProperty listProp, string resourcePath)
        {
            List<string> sofaOptions = LoadSofaOptions(resourcePath);

            if (sofaOptions == null || sofaOptions.Count == 0)
            {
                EditorGUILayout.HelpBox($"No SOFA files found in: Resources/{resourcePath}", MessageType.Warning);
                if (GUILayout.Button("Add Entry"))
                {
                    listProp.InsertArrayElementAtIndex(listProp.arraySize);
                }
                return;
            }

            for (int i = 0; i < listProp.arraySize; i++)
            {
                SerializedProperty element = listProp.GetArrayElementAtIndex(i);

                EditorGUILayout.BeginVertical("box");

                SerializedProperty sofaProp = element.FindPropertyRelative("sofaFile");

                int selected = Mathf.Max(0, sofaOptions.IndexOf(sofaProp.stringValue));
                int newSelected = EditorGUILayout.Popup("SOFA File", selected, sofaOptions.ToArray());
                sofaProp.stringValue = newSelected >= 0 ? sofaOptions[newSelected] : "";

                SerializedProperty propCopy = element.Copy();
                SerializedProperty endProp = propCopy.GetEndProperty();

                propCopy.NextVisible(true);

                while (!SerializedProperty.EqualContents(propCopy, endProp))
                {
                    if (propCopy.name != "sofaFile")
                    {
                        EditorGUILayout.PropertyField(propCopy, true);
                    }

                    if (!propCopy.NextVisible(false)) break;
                }

                if (GUILayout.Button("Remove"))
                {
                    listProp.DeleteArrayElementAtIndex(i);
                    break;
                }

                EditorGUILayout.EndVertical();
            }

            if (GUILayout.Button("Add Entry"))
            {
                listProp.InsertArrayElementAtIndex(listProp.arraySize);
            }
        }

        private List<string> LoadSofaOptions(string resourcePath)
        {
            return Resources.LoadAll<TextAsset>(resourcePath)
                .Select(sofa => sofa.name)
                .ToList();
        }

        private void DrawListenerModelIfExists()
        {
            if (listenerModels == null || listenerModels.arraySize == 0)
                return;

            SerializedProperty modelProp = listenerModels.GetArrayElementAtIndex(0);
            EditorGUILayout.LabelField("Listener Model", EditorStyles.boldLabel);
            DrawListenerModel(modelProp);
        }

        private void DrawListenerModel(SerializedProperty modelProp)
        {
            EditorGUI.indentLevel++;

            string[] hrtfOptions = GetSofaFileNames(hrtfResources);
            string[] nfcOptions = GetSofaFileNames(nfcFilterResources);

            SerializedProperty prop = modelProp.Copy();
            SerializedProperty endProp = prop.GetEndProperty();
            prop.NextVisible(true);

            while (!SerializedProperty.EqualContents(prop, endProp))
            {
                if (prop.name == "HRTFResourceIndex")
                {
                    int index = Mathf.Clamp(prop.intValue, 0, Mathf.Max(0, hrtfOptions.Length - 1));
                    index = EditorGUILayout.Popup("HRTF", index, hrtfOptions);
                    prop.intValue = index;
                }
                else if (prop.name == "NFCResourceIndex")
                {
                    int index = Mathf.Clamp(prop.intValue, 0, Mathf.Max(0, nfcOptions.Length - 1));
                    index = EditorGUILayout.Popup("NFC Filter", index, nfcOptions);
                    prop.intValue = index;
                }
                else
                {
                    EditorGUILayout.PropertyField(prop, true);
                }

                if (!prop.NextVisible(false)) break;
            }

            EditorGUI.indentLevel--;
        }

        private string[] GetSofaFileNames(SerializedProperty list)
        {
            var names = new List<string>();
            for (int i = 0; i < list.arraySize; i++)
            {
                var element = list.GetArrayElementAtIndex(i);
                var sofaProp = element.FindPropertyRelative("sofaFile");
                names.Add(!string.IsNullOrEmpty(sofaProp?.stringValue) ? sofaProp.stringValue : "<Missing>");
            }
            return names.ToArray();
        }
    }
}
