using UnityEngine;
using System.Linq;
using System;

namespace BRT
{
    [DisallowMultipleComponent]
    [RequireComponent(typeof(AudioSource))]
    public sealed class SpatializerSource : MonoBehaviour
    {
        private AudioSource audioSource;

        private string listenerModelId;
        private string listenerEnvironmentModelId;
        private int instanceId = -1;
        public int InstanceId => instanceId;

        [SerializeField] private bool enableDirectivity;

        private void OnValidate()
        {
            if (!Application.isPlaying) return;
            ApplyDirectivity();
        }

        private void Awake()
        {
            audioSource = GetComponent<AudioSource>();
        }

        private void Start()
        {
            RefreshInstanceId();
            SetDirectivityIndex(0);
            ApplyDirectivity();
        }

        public void SetDirectivityIndex(int index)
        {
            string file = BRTResourceCatalog.GetDirectivity(index);
            Logger.LogInfo(file);
            if (file == null) return;

            string path = BRTResourceCatalog.DirectivityResourceFolder + file;

            if (!ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
            {
                Logger.LogError($"[Directivity] Failed to extract: {file}");
                return;
            }

            NativePluginWrapper.BRTLoadSourceDirectivityTF(InstanceId.ToString(), fullPath);
        }

        private void ApplyDirectivity()
        {
            if (instanceId < 0) return;
            NativePluginWrapper.BRTSetSourceDirectivityEnabled(instanceId.ToString(), enableDirectivity);
        }
        
        private void RefreshInstanceId()
        {
            if (audioSource.GetSpatializerFloat((int)SpatializerParameter.InstanceId, out float idFloat))
                instanceId = Mathf.RoundToInt(idFloat);
            else
                Debug.LogWarning("[SpatialiserSource] Could not get instance ID from spatialiser plugin", this);
            
            Debug.Log("BRT: Instance id " + instanceId, this);
        }
    }
}
