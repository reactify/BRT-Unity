using UnityEngine;
using BRT.Log.Internal;

namespace BRT
{
    internal static class Logger
    {
        // -------- Public (managed) logging --------

        internal static void LogWarning(string message, Object context = null)
        {
            Write(LogLevel.Warning, message, "[BRT]", context);
        }

        internal static void LogError(string message, Object context = null)
        {
            Write(LogLevel.Error, message, "[BRT]", context);
        }

        internal static void LogInfo(string message, Object context = null)
        {
            Write(LogLevel.Info, message, "[BRT]", context);
        }

        private static void Write(LogLevel level, string message, string prefix, Object context)
        {
            string formatted = $"{prefix} {message}";

            switch (level)
            {
                case LogLevel.Error:
                    Debug.LogError(formatted, context);
                    break;

                case LogLevel.Warning:
                    Debug.LogWarning(formatted, context);
                    break;

                default:
                    Debug.Log(formatted, context);
                    break;
            }
        }

        // -------- Native integration --------

        internal static void Initialize()
        {
            LoggerDispatch.Initialize();
        }

        internal static void FlushNative()
        {
            while (LoggerDispatch.TryDequeue(out var level, out var message))
            {
                Write(level, message, "[BRT Native]", null);
            }
        }
    }
}