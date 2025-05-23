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
        }

    #if UNITY_EDITOR
        private void OnValidate()
        {
            if (!Application.isPlaying)
                Instance = this;
        }
    #endif

        private void Start()
        {
            foreach (var hrtf in configuration.hrtfResources)
            {
                string filePath = BRTConfiguration.HRTFResourceFolder + hrtf.sofaFile;
                if (ResourceExtractor.ExtractToPersistentDataPath(filePath, filePath, out string fullPath))
                {
                    Debug.Log("File ready at: " + fullPath);
                    NativePluginWrapper.BRTSpatializerCreateHRTF(fullPath);
                }
                else
                {
                    Debug.LogError("Failed to extract SOFA file.");
                }
                // NativePluginWrapper.BRTSpatializerCreateListenerModel(0, configuration.hrtfResources.IndexOf(hrtf).ToString());
            }

            foreach (var brir in configuration.brirResources)
            {
                string virtualPath = BRTConfiguration.BRIRResourceFolder + brir.sofaFile;
                // NativePluginWrapper.LoadBRIR(virtualPath);
            }
        }
    }
}