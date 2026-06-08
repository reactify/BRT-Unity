//
//  Logging.cpp
//  AudioPluginBRTUnity
//
//  Created by Ragnar Hrafnkelsson on 03/06/2026.
//

#include "Logging.h"
#include <atomic>
#include "AudioPluginBRTUnity.h"

static std::atomic<BRT_LogCallback> g_Callback = nullptr;
static std::atomic<bool> g_LoggingReady = false;

void SetLogCallback(BRT_LogCallback cb)
{
    g_Callback.store (cb, std::memory_order_release);
    g_LoggingReady.store (cb != nullptr, std::memory_order_release);
}

void BRT_Log (int level, const char* message)
{
    if (! g_LoggingReady.load(std::memory_order_acquire))
    {
        printf ("LOG DROPPED: %s\n", message);
        return;
    }

    auto cb = g_Callback.load (std::memory_order_acquire);

    if (cb)
        cb (level, message);
}

void BRT_Log (int level, std::string message)
{
    BRT_Log (level, message.c_str());
}
