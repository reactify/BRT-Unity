using System;
using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using UnityEngine;
using AOT;

namespace BRT.Log.Internal
{
    internal static class LoggerDispatch
    {
        private struct LogMessage
        {
            public LogLevel Level;
            public string Message;
        }

        private static readonly ConcurrentQueue<LogMessage> _queue = new();

        // Rooted delegate (required for AOT + GC safety)
        private static readonly LoggerInterop.LogCallback _callback = OnLog;

        internal static void Initialize()
        {
            LoggerInterop.SetLogCallback(_callback);
        }

        // Entry point from native (ANY thread, incl. audio)
        [MonoPInvokeCallback(typeof(LoggerInterop.LogCallback))]
        private static void OnLog(int level, IntPtr messagePtr)
        {
            string msg = PtrToString(messagePtr);

            _queue.Enqueue(new LogMessage
            {
                Level = (LogLevel)level,
                Message = msg
            });
        }

        internal static bool TryDequeue(out LogLevel level, out string message)
        {
            if (_queue.TryDequeue(out var log))
            {
                level = log.Level;
                message = log.Message;
                return true;
            }

            level = default;
            message = null;
            return false;
        }

        private static string PtrToString(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
                return string.Empty;

            return Marshal.PtrToStringAnsi(ptr);
        }
    }
}