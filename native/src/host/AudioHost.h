
#pragma once

#include "AudioEngine.h"
#include <portaudio.h>

class AudioHost
{
public:
    AudioHost();
    ~AudioHost();

    bool start(int sampleRate = 48000, int bufferSize = 256, int channels = 2);
    void stop();

    AudioEngine& engine() { return m_engine; }

private:
    static int paCallback(
        const void* input,
        void* output,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

private:
    PaStream* m_stream = nullptr;
    AudioEngine m_engine;
    int m_channels = 2;
};
