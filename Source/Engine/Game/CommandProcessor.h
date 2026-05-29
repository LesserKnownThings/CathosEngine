#pragma once

#include <cstdint>
#include <map>
#include <vector>

struct NetMessage;

class CommandProcessor
{
  public:
    static CommandProcessor& Get();

    void AddNetworkCommand(const NetMessage& cmd);

    void AddCommand(const NetMessage& cmd);
    std::vector<NetMessage> GetCommandsForTick(uint32_t tick);

  private:
    std::map<std::uint32_t, std::vector<NetMessage>> cmds;
};