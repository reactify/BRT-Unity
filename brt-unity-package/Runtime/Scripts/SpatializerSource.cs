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

        public int hrtfIndex = 0;
        public int brirIndex = 0;

        private void Awake()
        {
            audioSource = GetComponent<AudioSource>();
            if (configuration == null && BRTConfigurationLoader.Instance != null)
                configuration = BRTConfigurationLoader.Instance.configuration;
        }

        private void Start()
        {
            RefreshInstanceId();
            ApplyResourcesByIndex();
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
                Debug.LogError("[SpatialiserSource] Could not get instance ID from spatialiser plugin", this);
        }

        public void SetHrtfIndex(int index)
        {
            if (configuration == null || configuration.hrtfResources == null || index < 0 || index >= configuration.hrtfResources.Count)
                return;
            hrtfIndex = index;
            ApplyResourcesByIndex();
        }

        public void SetBrirIndex(int index)
        {
            if (configuration == null || configuration.brirResources == null || index < 0 || index >= configuration.brirResources.Count)
                return;
            brirIndex = index;
            ApplyResourcesByIndex();
        }

        public void ApplyResourcesByIndex()
        {
            if (instanceId < 0 || configuration == null)
                return;

            if (configuration.hrtfResources != null && hrtfIndex >= 0 && hrtfIndex < configuration.hrtfResources.Count)
            {
                var hrtf = configuration.hrtfResources[hrtfIndex];
                if (!string.IsNullOrEmpty(hrtf.sofaFile))
                    NativePluginWrapper.SetHrtfResource(instanceId, hrtf.sofaFile);
            }

            if (configuration.brirResources != null && brirIndex >= 0 && brirIndex < configuration.brirResources.Count)
            {
                var brir = configuration.brirResources[brirIndex];
                if (!string.IsNullOrEmpty(brir.sofaFile))
                    NativePluginWrapper.SetBrirResource(instanceId, brir.sofaFile);
            }
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
            Debug.Log("Listener model id: " + listenerModelId);

            var listenerEnvironmentModel = configuration.listenerEnvironmentModels?.FirstOrDefault();

            if (listenerEnvironmentModel == null)
            {
                Debug.LogError("No listener environment model found");
                return;
            }

            listenerEnvironmentModelId = listenerEnvironmentModel.ModelID;
            Debug.Log("Listener model id: " + listenerEnvironmentModelId);
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
