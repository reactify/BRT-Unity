namespace BRT
{
    using System.Collections.Generic;
    using UnityEngine;

    [CreateAssetMenu(menuName = "BRT/Configuration", fileName = "BRTConfiguration")]
    public class BRTConfiguration : ScriptableObject
    {
        public List<HRTFResource> hrtfResources = new();
        public List<BRIRResource> brirResources = new();
        public List<DirectivityResource> directivityResources = new();
        public List<NFCFilterResource> nfcFilterResources = new();

        public const string HRTFResourceFolder = "Data/HRTF/";
        public const string BRIRResourceFolder = "Data/BRIR/";
        public const string DirectivityResourceFolder = "Data/DirectivityTF/";
        public const string NFCFilterResourceFolder = "Data/SOSFilters/";
    }
}