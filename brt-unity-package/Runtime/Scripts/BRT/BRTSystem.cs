using UnityEngine;
using System.Collections.Generic;

namespace BRT
{
    public static class BRTSystem
    {
        private static bool _initialized;
        private static BRTConfiguration _config;

        private static int _dspBufferSize;

        // --------------------------------------------------------------------
        // Public API
        // --------------------------------------------------------------------

        public static BRTConfiguration ActiveConfig => _config;

        public static void Initialize(BRTConfiguration config = null)
        {
            if (_initialized)
                return;

            if (!IsSpatializerActive())
            {
                Logger.LogWarning("Spatializer not active, skipping initialization");
                return;
            }

            Logger.LogInfo("BRT.System.Initialize()");
            _initialized = true;

            AudioSettings.GetDSPBufferSize(out _dspBufferSize, out _);

            NativePluginWrapper.BRTSpatializerResetIfNeeded(
                AudioSettings.outputSampleRate,
                _dspBufferSize
            );

            // Always resolve a config and apply it
            if (config == null)
                config = Resources.Load<BRTConfiguration>("BRTDefault");

            SetConfig(config, reset: false);
        }

        public static void Shutdown()
        {
            if (!_initialized)
                return;

            Logger.LogInfo("BRT.System.Shutdown()");

            NativePluginWrapper.BRTSpatializerDestroy();

            _initialized = false;
            _config = null;
        }

        public static void ResetStatics()
        {
            _initialized = false;
            _config = null;
        }

        // --------------------------------------------------------------------
        // Config
        // --------------------------------------------------------------------

        public static void SetConfig(BRTConfiguration config, bool reset = true)
        {
            if (config == null)
            {
                Logger.LogError("SetConfig called with null config");
                return;
            }

            Logger.LogInfo($"SetConfig: {config.name}");

            if (_initialized && reset)
            {
                Logger.LogInfo("Resetting spatializer state");

                NativePluginWrapper.BRTSpatializerResetIfNeeded(
                    AudioSettings.outputSampleRate,
                    _dspBufferSize
                );
            }

            _config = config;

            if (_initialized)
                ApplyCurrentConfig();
        }

        public static void SetConfig(string resourcePath, bool reset = true)
        {
            var config = Resources.Load<BRTConfiguration>(resourcePath);

            if (config == null)
            {
                Logger.LogError($"Config not found at Resources/{resourcePath}");
                return;
            }

            SetConfig(config, reset);
        }

        // --------------------------------------------------------------------
        // Internal
        // --------------------------------------------------------------------

        private static void ApplyCurrentConfig()
        {
            if (_config == null)
            {
                Logger.LogError("ApplyCurrentConfig called with null config");
                return;
            }

            Logger.LogInfo($"Applying config: {_config.name}");

            var listener = _config.listenerModels?[0];
            var listenerEnv = _config.listenerEnvironmentModels?[0];

            if (listener == null || listenerEnv == null)
            {
                Logger.LogError("Invalid configuration");
                return;
            }

            // Listener
            NativePluginWrapper.BRTSpatializerCreateListener(listener.ListenerID);
            NativePluginWrapper.BRTSpatializerCreateListenerModel(0, listener.ModelID);
            NativePluginWrapper.BRTSpatializerConnectListenerModel(
                listener.ListenerID,
                listener.ModelID);

            // Environment
            NativePluginWrapper.BRTSpatializerCreateListenerModel(1, listenerEnv.ModelID);
            NativePluginWrapper.BRTSpatializerConnectListenerModel(
                listener.ListenerID,
                listenerEnv.ModelID);

            // Resources
            LoadResourceGroup(
                _config.hrtfResources,
                BRTConfiguration.HRTFResourceFolder,
                r => r.sofaFile,
                NativePluginWrapper.BRTSpatializerLoadHRTF,
                "HRTF"
            );

            LoadResourceGroup(
                _config.nfcFilterResources,
                BRTConfiguration.NFCFilterResourceFolder,
                r => r.sofaFile,
                NativePluginWrapper.BRTSpatializerLoadNearFieldCompensationFilter,
                "NFC"
            );

            LoadResourceGroup(
                _config.brirResources,
                BRTConfiguration.BRIRResourceFolder,
                r => r.sofaFile,
                NativePluginWrapper.BRTSpatializerLoadBRIR,
                "BRIR"
            );
        }

        private static void LoadResourceGroup<T>(
            IList<T> resources,
            string baseFolder,
            System.Func<T, string> fileSelector,
            System.Func<string, bool> loader,
            string label)
        {
            if (resources == null)
                return;

            foreach (var res in resources)
            {
                string sofaFile = fileSelector(res);
                string path = baseFolder + sofaFile;

                if (!ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
                {
                    Logger.LogError($"[{label}] Failed to extract: {sofaFile}");
                    continue;
                }

                if (!loader(fullPath))
                {
                    Logger.LogError($"[{label}] Failed to load: {sofaFile}");
                }
            }
        }

        private static bool IsSpatializerActive()
        {
#if UNITY_EDITOR || UNITY_STANDALONE || UNITY_IOS || UNITY_ANDROID
            var name = AudioSettings.GetSpatializerPluginName();
            return !string.IsNullOrEmpty(name);
#else
            return false;
#endif
        }
    }
}