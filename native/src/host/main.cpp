
#include "host/AudioHost.h"
#include <iostream>

int main()
{
    AudioHost host;

    if (!host.start(48000, 256, 2))
    {
        std::cerr << "Failed to start audio host\n";
        return -1;
    }

    std::cout << "Audio host running. Press Enter to exit...\n";
    std::cin.get();

    host.stop();
    return 0;
}
