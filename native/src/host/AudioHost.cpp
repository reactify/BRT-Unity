
#include "host/AudioHost.h"
#include <iostream>

AudioHost::AudioHost()
{
    Pa_Initialize();
}

AudioHost::~AudioHost()
{
    stop();
    Pa_Terminate();
}

bool AudioHost::start(int sampleRate, int bufferSize, int channels)
{
    m_channels = channels;

    m_engine.init(sampleRate, channels);

    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    outputParams.channelCount = channels;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency =
        Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &m_stream,
        nullptr,
        &outputParams,
        sampleRate,
        bufferSize,
        paNoFlag,
        &AudioHost::paCallback,
        this
    );

    if (err != paNoError)
    {
        std::cerr << "PortAudio open error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(m_stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudio start error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    return true;
}

void AudioHost::stop()
{
    if (m_stream)
    {
        Pa_StopStream(m_stream);
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
    }
}

int AudioHost::paCallback(
    const void*,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void* userData)
{
    auto* host = static_cast<AudioHost*>(userData);

    float* out = static_cast<float*>(outputBuffer);

    host->m_engine.process(out, framesPerBuffer, host->m_channels);

    return paContinue;
}
