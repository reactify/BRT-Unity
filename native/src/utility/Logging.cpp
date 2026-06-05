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

void SetLogCallback (BRT_LogCallback cb)
{
    g_Callback.store (cb, std::memory_order_release);
}

void BRT_Log(int level, const char* message)
{
    printf("BRT_Log called: %s\n", message);

    auto cb = g_Callback.load(std::memory_order_acquire);

    if (cb)
    {
        printf("Callback is valid\n");
        cb(level, message);
    }
    else
    {
        printf("Callback is NULL\n");
    }
}

void BRT_Log (int level, std::string message)
{
    BRT_Log (level, message.c_str());
}
