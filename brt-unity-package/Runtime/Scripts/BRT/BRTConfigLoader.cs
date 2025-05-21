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
                Debug.LogError("[BRTConfigurationLoader] Multiple instances detected. Only one is allowed.");
                Destroy(gameObject);
                return;
            }

            Instance = this;
        }

        private void Start()
        {
            foreach (var hrtf in configuration.hrtfResources)
            {
                string virtualPath = BRTConfiguration.HRTFResourceFolder + hrtf.sofaFile;
                NativePluginWrapper.LoadHRTF(virtualPath);
            }

            foreach (var brir in configuration.brirResources)
            {
                string virtualPath = BRTConfiguration.BRIRResourceFolder + brir.sofaFile;
                NativePluginWrapper.LoadBRIR(virtualPath);
            }
        }
    }
}