
#include <iostream>
#include "host/AudioHost.h"
#include "AudioPluginBRTUnity.h"
#include "BRTLibraryWrapper.h"

int main()
{
    AudioHost host;

    if (! host.start(48000, 256, 2))
    {
        std::cerr << "Failed to start audio host\n";
        return -1;
    }
    
    int length = 256;
    
    BRTSpatializerResetIfNeeded (44100, length);
    BRTCreateListener ("Listener_0");
    BRTCreateListenerModel (0, "Direct_Path");
    BRTConnectListenerModel ("Listener_0", "Direct_Path");
    BRTCreateListenerModel (1, "Reverb_Path");
    BRTConnectListenerModel ("Listener_0", "Reverb_Path");
    BRTCreateSoundSource ("Source_0");
    BRTLoadSourceDirectivityTF ("Source_0", "/Users/ragnaringi/Desktop/Cardioid_LP_30dB_512s_resampled10_normalized_fir_512.sofa");
    BRTSetSourceDirectivityEnabled ("Source_0", true);
    
    BRTClearGraph();
    BRTSpatializerDestroy();
    BRTSpatializerResetIfNeeded (44100, length);
    BRTSetSourceDirectivityEnabled ("Source_0", true);
    
    // BRTSptializer
    
    float inbuffer[1024];
    float outbuffer[1024];
    int inchannels = 2;
    int outchannels = 2;
    
    CMonoBuffer<float> inMonoBuffer (length);
    
    auto* brtInstance = BRTUnity::BRTLibraryWrapper::instance();
    
    auto soundSource = brtInstance->brtManager.GetSoundSource ("Source_0");
    
    // Transform input buffer
    for (size_t i = 0; i < length; i++)
        inMonoBuffer[i] = (inbuffer[i * 2] + inbuffer[i * 2 + 1]) / 2.0f;

    if (soundSource)
        soundSource->SetBuffer (inMonoBuffer);
    
    brtInstance->process (inbuffer, outbuffer, length, inchannels, outchannels);

    std::cout << "Audio host running. Press Enter to exit...\n";
    std::cin.get();

    host.stop();
    return 0;
}
