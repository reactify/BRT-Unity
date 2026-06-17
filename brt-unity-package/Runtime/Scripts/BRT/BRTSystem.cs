namespace BRT
{
    using System.Collections.Generic;
    using UnityEngine;

    public static class BRTSystem
    {
        private static bool _initialized;
        private static bool _listenerCreated;

        private static BRTConfiguration _activeConfig;

        private static int _dspBufferSize;

        // --------------------------------------------------------------------
        // Public API
        // --------------------------------------------------------------------

        public static BRTConfiguration ActiveConfig => _activeConfig;

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
            
            Logger.LogInfo("BRT intialised with sample rate: " + AudioSettings.outputSampleRate + " and buffer size: " + _dspBufferSize);

            BRTResourceCatalog.Rebuild();

            // If editor already selected a config, use it
            if (_activeConfig != null)
            {
                ApplyRuntimeConfig();
                return;
            }

            // Otherwise load default
            config ??= Resources.Load<BRTConfiguration>("BRTDefault");
            SetConfig(config, reset: false);
        }

        public static void Shutdown()
        {
            if (!_initialized)
                return;

            Logger.LogInfo("BRT.System.Shutdown()");

            NativePluginWrapper.BRTSpatializerDestroy();

            ClearState();
        }

        public static void ResetStatics()
        {
            ClearState();
            _activeConfig = null;
        }

        // --------------------------------------------------------------------
        // Config entry point
        // --------------------------------------------------------------------

        public static void SetConfig(BRTConfiguration config, bool reset = false)
        {
            if (config == null)
                return;
            
            _activeConfig = Object.Instantiate(config);
            _activeConfig.name = config.name;

            // If not running yet, stop here
            if (!_initialized)
                return;

            if (reset)
            {
                NativePluginWrapper.BRTSpatializerResetIfNeeded(
                    AudioSettings.outputSampleRate,
                    _dspBufferSize
                );

                _listenerCreated = false;
            }

            ApplyRuntimeConfig();
        }

        // --------------------------------------------------------------------
        // Runtime application
        // --------------------------------------------------------------------

        public static void ReapplyRuntimeConfig()
        {
            if (!_initialized || _activeConfig == null)
                return;

            ApplyRuntimeConfig();
        }

        private static void ApplyRuntimeConfig()
        {
            if (_activeConfig == null)
            {
                Logger.LogError("ApplyRuntimeConfig called with null config");
                return;
            }

            Logger.LogInfo($"Applying config: {_activeConfig.name}");

            var listener = _activeConfig.listenerModels?[0];
            var env = _activeConfig.listenerEnvironmentModels?[0];

            if (listener == null || env == null)
            {
                Logger.LogError("Invalid configuration");
                return;
            }

            // Only create listener once
            if (!_listenerCreated)
            {
                ApplyListener(listener, env);
                _listenerCreated = true;
            }

            // Always safe to update these
            SetHRTF(listener.HRTFResourceIndex);
            SetNFC(listener.NFCResourceIndex);
            SetBRIR(env.BRIRResourceIndex);
            
            NativePluginWrapper.BRTSetListenerModelParameters(listener.modelID, ref listener.parameters);
            NativePluginWrapper.BRTSetListenerModelParameters(env.modelID, ref env.parameters);
        }

        // --------------------------------------------------------------------
        // Listener setup
        // --------------------------------------------------------------------

        private static void ApplyListener(ListenerModel listener, ListenerEnvironmentModel env)
        {
            NativePluginWrapper.BRTSpatializerCreateListener(listener.listenerID);

            var listenerModelType =
                (int)(_activeConfig?.listenerModels?[0].parameters.type 
                      ?? ListenerModelType.DirectHRTFConvolutionModel);
            NativePluginWrapper.BRTSpatializerCreateListenerModel(listenerModelType, listener.modelID);
            NativePluginWrapper.BRTSpatializerConnectListenerModel(
                listener.listenerID,
                listener.modelID);

            listenerModelType =
                (int)(_activeConfig?.listenerEnvironmentModels?[0].parameters.type 
                      ?? ListenerModelType.DirectBRIRConvolutionModel);
            NativePluginWrapper.BRTSpatializerCreateListenerModel(listenerModelType, env.modelID);
            NativePluginWrapper.BRTSpatializerConnectListenerModel(
                env.listenerID,
                env.modelID);
        }

        // --------------------------------------------------------------------
        // Resource setters
        // --------------------------------------------------------------------

        public static void SetHRTF(int index)
        {
            var model = _activeConfig?.listenerModels?[0];
            if (model == null) return;

            model.HRTFResourceIndex = index;

            string file = BRTResourceCatalog.GetHRTF(index);
            if (file == null) return;

            string path = BRTResourceCatalog.HRTFResourceFolder + file;

            if (!ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
            {
                Logger.LogError($"[HRTF] Failed to extract: {file}");
                return;
            }

            if (!NativePluginWrapper.BRTSpatializerLoadHRTF(fullPath))
            {
                Logger.LogError("Failed to load HRTF");
            }
        }

        public static void SetNFC(int index)
        {
            var model = _activeConfig?.listenerModels?[0];
            if (model == null) return;

            model.NFCResourceIndex = index;
            
            string file = BRTResourceCatalog.GetNFC(index);
            if (file == null) return;
            
            string path = BRTResourceCatalog.NFCFilterResourceFolder + file;

            if (!ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
            {
                Logger.LogError($"[HRTF] Failed to extract: {file}");
                return;
            }

            if (!NativePluginWrapper.BRTSpatializerLoadNearFieldCompensationFilter(fullPath))
            {
                Logger.LogError("Failed to load NFC filter");
            }
        }

        public static void SetBRIR(int index)
        {
            var env = _activeConfig?.listenerEnvironmentModels?[0];
            if (env == null) return;

            env.BRIRResourceIndex = index;

            string file = BRTResourceCatalog.GetBRIR(index);
            if (file == null) return;

            string path = BRTResourceCatalog.BRIRResourceFolder + file;

            if (!ResourceExtractor.ExtractToPersistentDataPath(path, path, out string fullPath))
            {
                Logger.LogError($"[BRIR] Failed to extract: {file}");
                return;
            }

            if (!NativePluginWrapper.BRTSpatializerLoadBRIR(fullPath))
            {
                Logger.LogError("Failed to load BRIR");
            }
        }

        // --------------------------------------------------------------------
        // State
        // --------------------------------------------------------------------

        private static void ClearState()
        {
            _initialized = false;
            _listenerCreated = false;
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