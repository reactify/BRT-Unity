namespace BRT
{
    using UnityEngine;

    [System.Serializable]
    public class HRTFResource
    {
        public string sofaFile;

        [Range(1, 15)] public int spatialResolution = 5;
        [Range(31, 113)] public float headCircumference = 55f;
        public DelayForm delayForm = DelayForm.SofaDelays;

        public void OnSettingsChanged()
        {
            Debug.Log($"[HRTF] Settings changed: Resolution={spatialResolution}, Head={headCircumference}, DelayForm={delayForm}");
            // Hook into plugin logic here
        }
    }

    public enum DelayForm
    {
        SofaDelays,
        Woodworth
    }
}