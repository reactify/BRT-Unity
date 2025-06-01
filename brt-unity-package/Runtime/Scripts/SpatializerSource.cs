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

        [SerializeField] private BRTConfiguration configuration;
        public BRTConfiguration GetConfiguration()
        {
#if UNITY_EDITOR
            if (!Application.isPlaying && configuration == null)
            {
                var loader = FindFirstObjectByType<BRTConfigurationLoader>();
                if (loader != null)
                    configuration = loader.configuration;
            }
#endif
            return configuration;
        }

        public int directivityIndex = 0;

        private void Awake()
        {
            audioSource = GetComponent<AudioSource>();
            if (configuration == null && BRTConfigurationLoader.Instance != null)
                configuration = BRTConfigurationLoader.Instance.configuration;
        }

        private void Start()
        {
            RefreshInstanceId();
            InitialiseIdentifiers();
            CreateSoundSource();
            ConnectToListenerModel();
            ConnectToListenerEnvironmentModel();
        }

        public void RefreshInstanceId()
        {
            if (audioSource.GetSpatializerFloat((int)SpatializerParameter.InstanceId, out float idFloat))
                instanceId = Mathf.RoundToInt(idFloat);
            else
                Debug.Log("[SpatialiserSource] Could not get instance ID from spatialiser plugin", this);
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

        void InitialiseIdentifiers()
        {
            var listenerModel = configuration.listenerModels?.FirstOrDefault();

            if (listenerModel == null)
            {
                Debug.LogError("No listener model found");
                return;
            }

            listenerModelId = listenerModel.ModelID;

            var listenerEnvironmentModel = configuration.listenerEnvironmentModels?.FirstOrDefault();

            if (listenerEnvironmentModel == null)
            {
                Debug.LogError("No listener environment model found");
                return;
            }

            listenerEnvironmentModelId = listenerEnvironmentModel.ModelID;
        }

        void CreateSoundSource()
        {
            if (!NativePluginWrapper.BRTSpatializerCreateSoundSource(InstanceId.ToString()))
            {
                // Debug.LogError("Error connecting to listener model");
            }
        }

        void ConnectToListenerModel()
        {
            if (!NativePluginWrapper.BRTSpatializerConnectSoundSource(InstanceId.ToString(), listenerModelId))
            {
                Debug.LogError("Error connecting to listener model");
            }
        }
        
        void ConnectToListenerEnvironmentModel()
        {   
            if (!NativePluginWrapper.BRTSpatializerConnectSoundSource(InstanceId.ToString(), listenerEnvironmentModelId))
            {
                Debug.LogError("Error connecting to listener model");
            }
        }
    }
}
