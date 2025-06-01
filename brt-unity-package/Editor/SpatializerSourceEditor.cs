using UnityEditor;
using UnityEngine;

namespace BRT.Editor
{
    [CustomEditor(typeof(SpatializerSource))]
    public class SpatializerSourceEditor : UnityEditor.Editor
    {
        public override void OnInspectorGUI()
        {
            var spatializer = (SpatializerSource)target;
            var config = spatializer.GetConfiguration();

            if (config == null)
            {
                EditorGUILayout.HelpBox("No configuration found. Ensure BRTConfigurationLoader.Instance is set.", MessageType.Warning);
                return;
            }

            DrawDefaultInspectorWithout("directivityIndex");

            string[] directivityOptions = GetSofaNames(config.directivityResources);
            spatializer.directivityIndex = EditorGUILayout.Popup("Directivity", spatializer.directivityIndex, directivityOptions);

            if (GUI.changed)
            {
                EditorUtility.SetDirty(spatializer);
            }
        }

        private void DrawDefaultInspectorWithout(params string[] exclude)
        {
            var so = serializedObject;
            so.Update();

            var prop = so.GetIterator();
            bool enterChildren = true;
            while (prop.NextVisible(enterChildren))
            {
                if (prop.name == "m_Script" || System.Array.Exists(exclude, e => e == prop.name))
                    continue;
                EditorGUILayout.PropertyField(prop, true);
                enterChildren = false;
            }

            so.ApplyModifiedProperties();
        }

        private string[] GetSofaNames<T>(System.Collections.Generic.List<T> list) where T : class
        {
            if (list == null || list.Count == 0)
                return new[] { "(None)" };

            return list.ConvertAll(entry =>
            {
                var sofaProp = entry.GetType().GetField("sofaFile");
                return sofaProp != null ? (string)sofaProp.GetValue(entry) : "(Unknown)";
            }).ToArray();
        }
    }
}
