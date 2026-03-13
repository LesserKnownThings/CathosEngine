#include "Frontend.h"
#include "Netcode/NetworkManager.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <string>

Frontend& Frontend::Get()
{
    static Frontend instance{};
    return instance;
}

void Frontend::Init(bool inTerminal)
{
    NetworkManager& nm = NetworkManager::Get();

    if (inTerminal)
    {
        uint32_t option = 0;
        int32_t port = 0;

        std::cout << "Select: \n1. Host\n2. Join" << std::endl;
        std::cin >> option;
        if (option == 1)
        {
            std::cout << "Is listen server? (1: true, 0: false):" << std::endl;
            uint8_t isListen = 0;
            std::cin >> isListen;
            nm.HostServer(isListen, -1);
        }
        else if (option == 2)
        {
            std::cout << "Port: " << std::endl;
            std::cin >> port;
            nm.Join(std::format("127.0.0.1:{}", port));
        }
    }
}

void Frontend::Host()
{
}

void Frontend::Join(const std::string& addr)
{
}