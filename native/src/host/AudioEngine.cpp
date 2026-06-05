

#include "AudioEngine.h"
#include <cstring>

AudioEngine::AudioEngine() {}

AudioEngine::~AudioEngine() {}

void AudioEngine::init(double sampleRate, int channels)
{
    m_sampleRate = sampleRate;
    m_channels = channels;

    m_tempBuffer.resize(4096 * channels);
}

void AudioEngine::reset()
{
    std::fill(m_tempBuffer.begin(), m_tempBuffer.end(), 0.0f);
}

void AudioEngine::process(float* output, int numFrames, int numChannels)
{
    // Replace with your BRT processing pipeline

    for (int i = 0; i < numFrames * numChannels; i++)
    {
        output[i] = 0.0f; // placeholder silence
    }
}

