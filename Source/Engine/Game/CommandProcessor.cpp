#include "CommandProcessor.h"

#include "Netcode/Message.h"
#include "Netcode/NetworkManager.h"
#include <algorithm>
#include <cstdint>

// TODO check if 5 frames is not too much, fighting games usually use 3, but RTS should be fine wiht 5
constexpr uint32_t INPUT_DELAY = 5;

CommandProcessor& CommandProcessor::Get()
{
    static CommandProcessor instance{};
    return instance;
}

void CommandProcessor::AddNetworkCommand(const Message& cmd)
{
    uint32_t scheduleTick = cmd.tick + INPUT_DELAY;
    cmds[scheduleTick].push_back(cmd);

    uint8_t* data;
    int32_t dataSize;

    MessagePacker::PackMessage(cmd, data, dataSize);
    NetworkManager::Get().SendMessage(data, dataSize);
}

void CommandProcessor::AddCommand(const Message& cmd)
{
    uint32_t scheduleTick = cmd.tick + INPUT_DELAY;
    cmds[scheduleTick].push_back(cmd);
}

std::vector<Message> CommandProcessor::GetCommandsForTick(uint32_t tick)
{
    if (cmds.find(tick) == cmds.end())
        return {};

    std::vector<Message> outCmds = cmds[tick];

    std::sort(outCmds.begin(), outCmds.end(), [](const Message& a, const Message& b)
              { return a.player < b.player; });

    cmds.erase(tick);

    return outCmds;
}
