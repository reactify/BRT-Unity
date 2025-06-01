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

            LoadConfig();
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
            Debug.Log("Creating listener: " + listenerId);

            if (!NativePluginWrapper.BRTSpatializerCreateListener(listenerId))
                Debug.Log("Error creating listener");

            var listenerModelId = configuration.listenerModels[0].ModelID;
            if (!NativePluginWrapper.BRTSpatializerCreateListenerModel(0, listenerModelId))
                Debug.Log("Error creating listenerModel");

            if (!NativePluginWrapper.BRTSpatializerConnectListenerModel(listenerId, listenerModelId))
                Debug.Log("Error connecting listenerModel");

            var listenerEnvironmentModelId = configuration.listenerEnvironmentModels[0].ModelID;
            if (!NativePluginWrapper.BRTSpatializerCreateListenerModel(1, listenerEnvironmentModelId))
                Debug.Log("Error creating listenerEnvironmentModel");

            if (!NativePluginWrapper.BRTSpatializerConnectListenerModel(listenerId, listenerEnvironmentModelId))
                Debug.Log("Error connecting listenerModel");

            foreach (var hrtf in configuration.hrtfResources)
            {
                string filePath = BRTConfiguration.HRTFResourceFolder + hrtf.sofaFile;
                if (ResourceExtractor.ExtractToPersistentDataPath(filePath, filePath, out string fullPath))
                {
                    Debug.Log("File ready at: " + fullPath);
                    NativePluginWrapper.BRTSpatializerLoadHRTF(fullPath);
                }
                else
                {
                    Debug.LogError("Failed to extract SOFA file.");
                }
            }

            if (!NativePluginWrapper.BRTSpatializerSetHRTF(0)) // TODO: Use resource index
                Debug.Log("Error setting HRTF");

            foreach (var brir in configuration.brirResources)
            {
                string virtualPath = BRTConfiguration.BRIRResourceFolder + brir.sofaFile;
                // NativePluginWrapper.LoadBRIR(virtualPath);
            }

            foreach (var brir in configuration.brirResources)
            {
                string filePath = BRTConfiguration.BRIRResourceFolder + brir.sofaFile;
                if (ResourceExtractor.ExtractToPersistentDataPath(filePath, filePath, out string fullPath))
                {
                    Debug.Log("File ready at: " + fullPath);
                    NativePluginWrapper.BRTSpatializerLoadBRIR(fullPath);
                }
                else
                {
                    Debug.LogError("Failed to extract SOFA file.");
                }
            }

            if (!NativePluginWrapper.BRTSpatializerSetBRIR(0)) // TODO: Use resource index
                Debug.Log("Error setting HRTF");
        }
    }
}