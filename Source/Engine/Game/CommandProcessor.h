#pragma once

#include <cstdint>
#include <map>
#include <vector>

struct Message;

class CommandProcessor
{
  public:
    static CommandProcessor& Get();

    void AddNetworkCommand(const Message& cmd);

    void AddCommand(const Message& cmd);
    std::vector<Message> GetCommandsForTick(uint32_t tick);

  private:
    std::map<std::uint32_t, std::vector<Message>> cmds;
};