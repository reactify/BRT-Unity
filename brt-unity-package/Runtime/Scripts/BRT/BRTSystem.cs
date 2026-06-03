using UnityEngine;
using System.Collections.Generic;

namespace BRT
{
    public static class BRTSystem
    {
        private static bool _initialized;
        private static BRTConfiguration _config;

        // Track active spatializer sources
        private static readonly HashSet<int> _activeSources = new HashSet<int>();

        // --------------------------------------------------------------------
        // Public API
        // --------------------------------------------------------------------

        public static void Initialize()
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

            AudioSettings.GetDSPBufferSize(out int dspBufferSize, out _);

            NativePluginWrapper.BRTSpatializerResetIfNeeded(
                AudioSettings.outputSampleRate,
                dspBufferSize
            );

            LoadConfig();
            ApplyConfig();
        }

        public static void Shutdown()
        {
            if (!_initialized)
                return;

            Logger.LogInfo("BRT.System.Shutdown()");

            // Optional if your native plugin supports it
            NativePluginWrapper.BRTSpatializerDestroy();

            _activeSources.Clear();
            _initialized = false;
        }

        public static void ResetStatics()
        {
            _initialized = false;
            _config = null;
            _activeSources.Clear();
        }

        public static BRTConfiguration GetConfig()
        {
            if (_config == null)
                _config = Resources.Load<BRTConfiguration>("BRTDefault");

            return _config;
        }

        // --------------------------------------------------------------------
        // Source lifecycle
        // --------------------------------------------------------------------

        public static void RegisterSource(int instanceId)
        {
            if (!_initialized || instanceId < 0)
                return;

            if (_activeSources.Contains(instanceId))
                return;

            _activeSources.Add(instanceId);

            var cfg = GetConfig();
            if (cfg == null)
                return;

            var listenerModel = cfg.listenerModels?[0];
            var envModel = cfg.listenerEnvironmentModels?[0];

            if (listenerModel != null)
            {
                if (!NativePluginWrapper.BRTSpatializerConnectSoundSource(
                        instanceId.ToString(),
                        listenerModel.ModelID))
                {
                    Logger.LogError("Failed to connect source to listener model");
                }
            }

            if (envModel != null)
            {
                if (!NativePluginWrapper.BRTSpatializerConnectSoundSource(
                        instanceId.ToString(),
                        envModel.ModelID))
                {
                    Logger.LogError("Failed to connect source to environment model");
                }
            }
        }

        public static void UnregisterSource(int instanceId)
        {
            if (instanceId < 0)
                return;

            if (_activeSources.Remove(instanceId))
            {
                // Optional native cleanup if available
                // NativePluginWrapper.BRTSpatializerRemoveSoundSource(instanceId.ToString());
            }
        }

        public static void SetSourceDirectivity(int instanceId, int directivityIndex)
        {
            if (!_initialized || instanceId < 0)
                return;

            var cfg = GetConfig();
            if (cfg == null)
                return;

            if (cfg.directivityResources == null ||
                directivityIndex < 0 ||
                directivityIndex >= cfg.directivityResources.Count)
                return;

            var directivity = cfg.directivityResources[directivityIndex];

            Logger.LogInfo($"Set directivity {directivity.sofaFile} for source {instanceId}");

            // TODO: hook into native API when implemented
        }

        // --------------------------------------------------------------------
        // Internal
        // --------------------------------------------------------------------

        private static void LoadConfig()
        {
            _config = Resources.Load<BRTConfiguration>("BRTDefault");

            if (_config == null)
            {
                Logger.LogError("Default config not found (BRTDefault)");
                return;
            }
        }

        private static void ApplyConfig()
        {
            if (_config == null)
                return;

            var listener = _config.listenerModels?[0];
            var listenerEnv = _config.listenerEnvironmentModels?[0];

            if (listener == null || listenerEnv == null)
            {
                Logger.LogError("Invalid configuration");
                return;
            }

            // Create listener
            if (!NativePluginWrapper.BRTSpatializerCreateListener(listener.ListenerID))
                Logger.LogError("Error creating listener");

            // Listener model
            if (!NativePluginWrapper.BRTSpatializerCreateListenerModel(0, listener.ModelID))
                Logger.LogError("Error creating listener model");

            if (!NativePluginWrapper.BRTSpatializerConnectListenerModel(
                    listener.ListenerID,
                    listener.ModelID))
                Logger.LogError("Error connecting listener model");

            // Environment model
            if (!NativePluginWrapper.BRTSpatializerCreateListenerModel(1, listenerEnv.ModelID))
                Logger.LogError("Error creating environment model");

            if (!NativePluginWrapper.BRTSpatializerConnectListenerModel(
                    listener.ListenerID,
                    listenerEnv.ModelID))
                Logger.LogError("Error connecting environment model");

            // HRTFs
            foreach (var hrtf in _config.hrtfResources)
            {
                string path = BRTConfiguration.HRTFResourceFolder + hrtf.sofaFile;

                if (ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
                {
                    if (!NativePluginWrapper.BRTSpatializerLoadHRTF(fullPath))
                        Logger.LogError("Failed to load HRTF");
                }
            }

            // NFC
            foreach (var nfc in _config.nfcFilterResources)
            {
                string path = BRTConfiguration.NFCFilterResourceFolder + nfc.sofaFile;

                if (ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
                {
                    if (!NativePluginWrapper.BRTSpatializerLoadNearFieldCompensationFilter(fullPath))
                        Logger.LogError("Failed to load NFC");
                }
            }

            // BRIR
            foreach (var brir in _config.brirResources)
            {
                string path = BRTConfiguration.BRIRResourceFolder + brir.sofaFile;

                if (ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
                {
                    if (!NativePluginWrapper.BRTSpatializerLoadBRIR(fullPath))
                        Logger.LogError("Failed to load BRIR");
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