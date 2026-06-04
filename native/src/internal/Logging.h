//
//  Logging.h
//  AudioPluginBRTUnity
//
//  Created by Ragnar Hrafnkelsson on 03/06/2026.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*BRT_LogCallback)(int level, const char* message);

void SetLogCallback (BRT_LogCallback cb);
void BRT_Log (int level, const char* message);

#ifdef __cplusplus
}

#include <string>
void BRT_Log (int level, std::string message);

#endif
