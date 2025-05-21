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

        private void OnEnable()
        {
            hrtfResources = serializedObject.FindProperty("hrtfResources");
            brirResources = serializedObject.FindProperty("brirResources");
        }

        public override void OnInspectorGUI()
        {
            serializedObject.Update();

            EditorGUILayout.LabelField("HRTF Resources", EditorStyles.boldLabel);
            DrawResourceList(hrtfResources, BRTConfiguration.HRTFResourceFolder);

            EditorGUILayout.Space(10);

            EditorGUILayout.LabelField("BRIR Resources", EditorStyles.boldLabel);
            DrawResourceList(brirResources, BRTConfiguration.BRIRResourceFolder);

            serializedObject.ApplyModifiedProperties();
        }

        private void DrawResourceList(SerializedProperty listProp, string resourcePath)
        {
            for (int i = 0; i < listProp.arraySize; i++)
            {
                SerializedProperty element = listProp.GetArrayElementAtIndex(i);

                EditorGUILayout.BeginVertical("box");
                SerializedProperty sofaProp = element.FindPropertyRelative("sofaFile");
                List<string> sofaOptions = LoadSofaOptions(resourcePath);
                int selected = Mathf.Max(0, sofaOptions.IndexOf(sofaProp.stringValue));
                int newSelected = EditorGUILayout.Popup("SOFA File", selected, sofaOptions.ToArray());
                sofaProp.stringValue = newSelected >= 0 ? sofaOptions[newSelected] : "";

                // Show extra settings if this is an HRTF resource
                if (element.FindPropertyRelative("spatialResolution") != null)
                {
                    EditorGUILayout.PropertyField(element.FindPropertyRelative("spatialResolution"));
                    EditorGUILayout.PropertyField(element.FindPropertyRelative("headCircumference"));
                    EditorGUILayout.PropertyField(element.FindPropertyRelative("delayForm"));
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
    }
}