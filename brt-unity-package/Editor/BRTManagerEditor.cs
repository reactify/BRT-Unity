namespace BRT.Editor
{
    using UnityEditor;
    using UnityEngine;

    [CustomEditor(typeof(BRTManager))]
    public class BRTManagerEditor : Editor
    {
        private Editor configEditor;

        public override void OnInspectorGUI()
        {
            serializedObject.Update();

            var configProp = serializedObject.FindProperty("configuration");
            EditorGUILayout.PropertyField(configProp);

            if (configProp.objectReferenceValue != null)
            {
                if (configEditor == null || configEditor.target != configProp.objectReferenceValue)
                    configEditor = CreateEditor(configProp.objectReferenceValue);

                EditorGUILayout.Space();
                configEditor.OnInspectorGUI();
            }

            serializedObject.ApplyModifiedProperties();
        }
    }
}
