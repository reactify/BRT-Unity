namespace BRT.Editor
{
    using UnityEditor;
    using UnityEngine;

    [CustomEditor(typeof(BRTConfiguration))]
    public class BRTConfigurationEditor : UnityEditor.Editor
    {
        private SerializedProperty listenerModels;
        private SerializedProperty listenerEnvironmentModels;

        // --------------------------------------------------------------------
        // INIT
        // --------------------------------------------------------------------

        private void OnEnable()
        {
            listenerModels = serializedObject.FindProperty("listenerModels");
            listenerEnvironmentModels = serializedObject.FindProperty("listenerEnvironmentModels");

            BRTResourceCatalog.Ensure();
        }

        // --------------------------------------------------------------------
        // INSPECTOR
        // --------------------------------------------------------------------

        public override void OnInspectorGUI()
        {
            serializedObject.Update();

            EditorGUILayout.LabelField("BRT Configuration", EditorStyles.boldLabel);
            EditorGUILayout.Space(10);

            DrawListenerModels();

            serializedObject.ApplyModifiedProperties();
        }

        // --------------------------------------------------------------------
        // LISTENER MODELS
        // --------------------------------------------------------------------

        private void DrawListenerModels()
        {
            if (listenerModels == null || listenerModels.arraySize == 0)
                return;

            EditorGUILayout.LabelField("Listener Model", EditorStyles.boldLabel);
            DrawListenerModel(listenerModels.GetArrayElementAtIndex(0));

            EditorGUILayout.Space(10);

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
                switch (prop.name)
                {
                    case "HRTFResourceIndex":
                        DrawPopup("HRTF", prop, BRTResourceCatalog.HRTF);
                        break;

                    case "NFCResourceIndex":
                        DrawPopup("NFC Filter", prop, BRTResourceCatalog.NFC);
                        break;

                    case "BRIRResourceIndex":
                        DrawPopup("BRIR", prop, BRTResourceCatalog.BRIR);
                        break;

                    default:
                        EditorGUILayout.PropertyField(prop, true);
                        break;
                }

                if (!prop.NextVisible(false))
                    break;
            }

            EditorGUI.indentLevel--;
        }

        // --------------------------------------------------------------------
        // POPUP
        // --------------------------------------------------------------------

        private void DrawPopup(string label, SerializedProperty prop, string[] options)
        {
            if (options == null || options.Length == 0)
            {
                EditorGUILayout.LabelField(label, "No resources found");
                return;
            }

            int index = Mathf.Clamp(prop.intValue, 0, options.Length - 1);

            int newIndex = EditorGUILayout.Popup(label, index, options);

            if (newIndex != prop.intValue)
            {
                prop.intValue = newIndex;
            }
        }
    }
}