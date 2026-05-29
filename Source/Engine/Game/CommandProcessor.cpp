#include "CommandProcessor.h"

#include "Netcode/NetMessage.h"
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

void CommandProcessor::AddNetworkCommand(const NetMessage& cmd)
{
    uint32_t scheduleTick = cmd.tick + INPUT_DELAY;
    cmds[scheduleTick].push_back(cmd);

    uint8_t* data;
    int32_t dataSize;

    MessagePacker::PackMessage(cmd, data, dataSize);
    NetworkManager::Get().SendMessage(data, dataSize);
}

void CommandProcessor::AddCommand(const NetMessage& cmd)
{
    uint32_t scheduleTick = cmd.tick + INPUT_DELAY;
    cmds[scheduleTick].push_back(cmd);
}

std::vector<NetMessage> CommandProcessor::GetCommandsForTick(uint32_t tick)
{
    if (cmds.find(tick) == cmds.end())
        return {};

    std::vector<NetMessage> outCmds = cmds[tick];

    std::sort(outCmds.begin(), outCmds.end(), [](const NetMessage& a, const NetMessage& b)
              { return a.player < b.player; });

    cmds.erase(tick);

    return outCmds;
}
