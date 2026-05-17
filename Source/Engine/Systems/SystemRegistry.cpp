#include "SystemRegistry.h"
#include <cstdint>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

SystemRegistry& SystemRegistry::Get()
{
    static SystemRegistry instance;
    return instance;
}

void SystemRegistry::Init(Registry* registry, CommandBuffer& cmd)
{
    Bake();

    for (auto& info : sortedSystems)
    {
        if (info.creator)
        {
            info.creator();
            info.initFunc(registry, cmd);
        }
    }
}

void SystemRegistry::Bake()
{
    std::unordered_map<std::type_index, std::vector<std::type_index>> adjacencyList;
    std::unordered_map<std::type_index, int32_t> inDegree;

    std::unordered_map<std::type_index, int> orderIndex;

    for (auto& info : infos)
    {
        orderIndex[info.id] = info.sortIndex;
    }

    for (auto& info : infos)
    {
        for (std::type_index before : info.after)
        {
            adjacencyList[before].push_back(info.id);
            inDegree[info.id]++;
        }

        for (std::type_index after : info.before)
        {
            adjacencyList[info.id].push_back(after);
            inDegree[after]++;
        }
    }

    auto cmp = [&](const std::type_index& a, const std::type_index& b)
    {
        return orderIndex[a] > orderIndex[b];
    };

    std::priority_queue<std::type_index, std::vector<std::type_index>, decltype(cmp)> zeroDegreeQueue(cmp);

    for (auto& info : infos)
    {
        if (inDegree[info.id] == 0)
        {
            zeroDegreeQueue.push(info.id);
        }
    }

    while (!zeroDegreeQueue.empty())
    {
        std::type_index current = zeroDegreeQueue.top();
        zeroDegreeQueue.pop();

        // This is not as bad as it looks, the engine doesn't have that many systems
        for (auto& info : infos)
        {
            if (info.id == current)
            {
                sortedSystems.emplace_back(info.id, info.phase, SystemFunc(), SyncSystemFunc(), InitFunc(), info.creator);
                break;
            }
        }

        for (std::type_index n : adjacencyList[current])
        {
            inDegree[n]--;
            if (inDegree[n] == 0)
            {
                zeroDegreeQueue.push(n);
            }
        }
    }
}

void SystemRegistry::Run(Registry* registry, CommandBuffer& cmd, SystemPhase phase)
{
    for (auto& sys : sortedSystems)
    {
        if (sys.phase == phase)
        {
            sys.func(registry, cmd);
        }
    }
}

void SystemRegistry::RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick, SystemPhase phase)
{
    for (auto& sys : sortedSystems)
    {
        if (sys.phase == phase)
        {
            sys.syncFunc(registry, cmd, tick);
        }
    }
}