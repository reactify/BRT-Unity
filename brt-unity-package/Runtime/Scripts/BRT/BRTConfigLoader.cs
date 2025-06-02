namespace BRT
{
    using UnityEngine;

    public class BRTConfigurationLoader : MonoBehaviour
    {
        public static BRTConfigurationLoader Instance { get; private set; }

        public BRTConfiguration configuration;

        private void Awake()
        {
            if (Instance != null && Instance != this)
            {
                Destroy(gameObject);
                return;
            }

            Instance = this;
            DontDestroyOnLoad(gameObject);

            Initialize();
        }

        private void Start()
        {
            Initialize();
            LoadConfig();
        }

        private void Initialize()
        {
            AudioSettings.GetDSPBufferSize(out int dspBufferSize, out _);
            NativePluginWrapper.BRTSpatialiserResetIfNeeded(AudioSettings.outputSampleRate, dspBufferSize);
            Debug.Log("BRT: Output Sample Rate " + AudioSettings.outputSampleRate);
        }

    #if UNITY_EDITOR
        private void OnValidate()
        {
            if (!Application.isPlaying)
                Instance = this;
        }
#endif

        private void LoadConfig()
        {
            var listenerId = configuration.listenerModels[0].ListenerID;
            Debug.Log("BRT: Creating listener: " + listenerId);

            if (!NativePluginWrapper.BRTSpatializerCreateListener(listenerId))
                Debug.LogError("BRT: Error creating listener");

            var listenerModelId = configuration.listenerModels[0].ModelID;
            Debug.Log("BRT: Creating listener model " + listenerModelId);
            if (!NativePluginWrapper.BRTSpatializerCreateListenerModel(0, listenerModelId))
                Debug.LogError("Error creating listenerModel " + listenerModelId);

            if (!NativePluginWrapper.BRTSpatializerConnectListenerModel(listenerId, listenerModelId))
                Debug.LogError("BRT: Error connecting listenerModel");

            var listenerEnvironmentModelId = configuration.listenerEnvironmentModels[0].ModelID;
            Debug.Log("BRT: Creating listener model " + listenerEnvironmentModelId);
            if (!NativePluginWrapper.BRTSpatializerCreateListenerModel(1, listenerEnvironmentModelId))
                Debug.LogError("BRT: Error creating listenerEnvironmentModel");

            Debug.Log("BRT: Connecting listener model " + listenerEnvironmentModelId + " to listener " + listenerId);
            if (!NativePluginWrapper.BRTSpatializerConnectListenerModel(listenerId, listenerEnvironmentModelId))
                Debug.Log("BRT: Error connecting listenerModel");

            foreach (var hrtf in configuration.hrtfResources)
            {
                string filePath = BRTConfiguration.HRTFResourceFolder + hrtf.sofaFile;
                Debug.Log("BRT: Attempting to load HRTF: " + filePath);
                if (ResourceExtractor.ExtractToPersistentDataPath(filePath, filePath, out string fullPath))
                {
                    Debug.Log("BRT: File ready at: " + fullPath);
                    NativePluginWrapper.BRTSpatializerLoadHRTF(fullPath);
                }
                else
                {
                    Debug.LogError("BRT: Failed to extract SOFA file.");
                }
            }

            if (!NativePluginWrapper.BRTSpatializerSetHRTF(0)) // TODO: Use resource index
                Debug.LogError("BRT: Error setting HRTF");


            foreach (var brir in configuration.brirResources)
            {
                string filePath = BRTConfiguration.BRIRResourceFolder + brir.sofaFile;
                Debug.Log("BRT: Attempting to load BRIR: " + filePath);
                if (ResourceExtractor.ExtractToPersistentDataPath(filePath, filePath, out string fullPath))
                {
                    Debug.Log("BRT: File ready at: " + fullPath);
                    NativePluginWrapper.BRTSpatializerLoadBRIR(fullPath);
                }
                else
                {
                    Debug.LogError("BRT: Failed to extract SOFA file.");
                }
            }

            if (!NativePluginWrapper.BRTSpatializerSetBRIR(0)) // TODO: Use resource index
                Debug.LogError("BRT: Error setting BRIR");
        }
    }
}