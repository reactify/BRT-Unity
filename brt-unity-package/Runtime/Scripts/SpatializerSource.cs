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
        public int directivityIndex = 0;

        [SerializeField] private BRTConfiguration configuration;
        public BRTConfiguration GetConfiguration()
        {
            return BRTSystem.ActiveConfig;
        }

        [SerializeField] private bool enableDirectivity;

        private void OnValidate()
        {
            EnableDirectivity(enableDirectivity);
        }

        private void Awake()
        {
            audioSource = GetComponent<AudioSource>();
            configuration = GetConfiguration();
        }

        private void Start()
        {
            RefreshInstanceId();
            InitialiseIdentifiers();
            // ConnectToListenerModel();
            // ConnectToListenerEnvironmentModel();
            SetDirectivityIndex(0);
            EnableDirectivity(enableDirectivity);
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

        public void EnableDirectivity(bool enable)
        {
            NativePluginWrapper.BRTSetSourceDirectivityEnabled(InstanceId.ToString(), enable);
        }
        
        private void RefreshInstanceId()
        {
            if (audioSource.GetSpatializerFloat((int)SpatializerParameter.InstanceId, out float idFloat))
                instanceId = Mathf.RoundToInt(idFloat);
            else
                Debug.LogWarning("[SpatialiserSource] Could not get instance ID from spatialiser plugin", this);
            
            Debug.Log("BRT: Instance id " + instanceId, this);
        }

        private void InitialiseIdentifiers()
        {
            var listenerModel = configuration.listenerModels?.FirstOrDefault();

            if (listenerModel == null)
            {
                Debug.LogError("BRT: No listener model found");
                return;
            }

            listenerModelId = listenerModel.modelID;

            var listenerEnvironmentModel = configuration.listenerEnvironmentModels?.FirstOrDefault();

            if (listenerEnvironmentModel == null)
            {
                Debug.LogError("BRT: No listener environment model found");
                return;
            }

            listenerEnvironmentModelId = listenerEnvironmentModel.modelID;
        }

        // NOTE: Sound source is created internally by the spatializer plugin
        // We leave this here for possible future use
        private void CreateSoundSource()
        {
            if (!NativePluginWrapper.BRTSpatializerCreateSoundSource(InstanceId.ToString()))
                Debug.LogError("BRT: Error creating sound source " + InstanceId.ToString());
        }

        private void ConnectToListenerModel()
        {
            if (!NativePluginWrapper.BRTSpatializerConnectSoundSource(InstanceId.ToString(), listenerModelId))
            {
                Debug.LogError("BRT: Error connecting sound source " + InstanceId.ToString() + " to listener model " + listenerModelId);
            }
        }
        
        private void ConnectToListenerEnvironmentModel()
        {   
            if (!NativePluginWrapper.BRTSpatializerConnectSoundSource(InstanceId.ToString(), listenerEnvironmentModelId))
                Debug.LogError("BRT: Error connecting sound source " + InstanceId.ToString() + " to listener environment model " + listenerEnvironmentModelId);
        }
    }
}
