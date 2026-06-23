namespace BRT
{
    using System.Collections.Generic;
    using UnityEngine;
#if UNITY_EDITOR
    using UnityEditor;
#endif

    [CreateAssetMenu(menuName = "BRT/Configuration", fileName = "BRTConfiguration")]
    public class BRTConfiguration : ScriptableObject
    {
        [SerializeField] public List<ListenerModel> listenerModels = new();
        [SerializeField] public List<ListenerEnvironmentModel> listenerEnvironmentModels = new();
        [SerializeField] public EnvironmentModel environmentModel;

        private void OnEnable()
        {
            if (listenerModels == null || listenerModels.Count == 0)
            {
                listenerModels.Add(new ListenerModel());
#if UNITY_EDITOR
                EditorUtility.SetDirty(this);
#endif
            }

            if (listenerEnvironmentModels == null || listenerEnvironmentModels.Count == 0)
            {
                listenerEnvironmentModels.Add(new ListenerEnvironmentModel());
#if UNITY_EDITOR
                EditorUtility.SetDirty(this);
#endif
            }
        }
    }
}