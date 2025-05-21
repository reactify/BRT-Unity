using UnityEngine;

namespace BRT
{
    [DisallowMultipleComponent]
    [RequireComponent(typeof(AudioSource))]
    public class SpatializerSource : MonoBehaviour
    {
        private AudioSource audioSource;
        private int instanceID = -1;

        public int InstanceID => instanceID;

        void Awake()
        {
            audioSource = GetComponent<AudioSource>();
        }

        void Start()
        {
            RefreshInstanceID();
        }

        public void RefreshInstanceID()
        {
            if (audioSource.GetSpatializerFloat((int)SpatializerParameter.InstanceId, out float idFloat))
            {
                instanceID = Mathf.RoundToInt(idFloat);
#if UNITY_EDITOR
                Debug.Log($"[SpatializerSource] Instance ID assigned: {instanceID}", this);
#endif
            }
            else
            {
                Debug.LogWarning("[SpatializerSource] Failed to get instance ID from Spatializer plugin", this);
            }
        }

        public void SetSpatializerFloat(int index, float value)
        {
            audioSource.SetSpatializerFloat(index, value);
        }

        public float GetSpatializerFloat(int index)
        {
            if (audioSource.GetSpatializerFloat(index, out float value))
                return value;

            Debug.LogWarning($"[SpatializerSource] Failed to get float at index {index}", this);
            return 0f;
        }
    }
}
