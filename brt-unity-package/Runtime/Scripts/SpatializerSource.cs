using UnityEngine;

namespace BRT
{
    [DisallowMultipleComponent]
    [RequireComponent(typeof(AudioSource))]
    public sealed class SpatializerSource : MonoBehaviour
    {
        private AudioSource audioSource;
        private int instanceId = -1;

        public int InstanceId => instanceId;

        [SerializeField] private BRTConfiguration configuration;
        public BRTConfiguration GetConfiguration()
        {
        #if UNITY_EDITOR
            if (!Application.isPlaying && configuration == null)
            {
                var loader = Object.FindObjectOfType<BRTConfigurationLoader>();
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
        }

        public void RefreshInstanceId()
        {
            if (audioSource.GetSpatializerFloat((int)SpatializerParameter.InstanceId, out float idFloat))
                instanceId = Mathf.RoundToInt(idFloat);
            else
                Debug.LogWarning("[SpatialiserSource] Could not get instance ID from spatialiser plugin", this);
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
    }
}
