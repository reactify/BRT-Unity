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
            return BRTSystem.GetConfig();
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
            // CreateSoundSource();
            ConnectToListenerModel();
            ConnectToListenerEnvironmentModel();
        }

        public void SetDirectivityIndex(int index)
        {
            if (instanceId < 0 || configuration == null)
                return;

            if (configuration.directivityResources == null || index < 0 || index >= configuration.directivityResources.Count)
                return;

            directivityIndex = index;

            var directivity = configuration.directivityResources[directivityIndex];
            Debug.Log("TODO: Load directivity");
            // if (!string.IsNullOrEmpty(directivity.sofaFile))
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

            listenerModelId = listenerModel.ModelID;

            var listenerEnvironmentModel = configuration.listenerEnvironmentModels?.FirstOrDefault();

            if (listenerEnvironmentModel == null)
            {
                Debug.LogError("BRT: No listener environment model found");
                return;
            }

            listenerEnvironmentModelId = listenerEnvironmentModel.ModelID;
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
