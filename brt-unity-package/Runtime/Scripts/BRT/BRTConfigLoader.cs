namespace BRT
{
    using UnityEngine;

    public class BRTConfigurationLoader : MonoBehaviour
    {
        public BRTConfiguration configuration;

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