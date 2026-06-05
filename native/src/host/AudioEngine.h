
#pragma once
#include <vector>
#include <cstdint>

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    void init(double sampleRate, int channels);
    void reset();

    // main processing entry point (shared by Unity + Host)
    void process(float* output, int numFrames, int numChannels);

private:
    double m_sampleRate = 48000.0;
    int m_channels = 2;

    // Example internal buffers/state
    std::vector<float> m_tempBuffer;
};
