#pragma once

#include "Callme/CallMe.h"
#include "Systems/ISystem.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <vector>

class Registry;
class CommandBuffer;

using SystemFunc = CallMe::Delegate<void(Registry*, CommandBuffer&)>;
using SyncSystemFunc = CallMe::Delegate<void(Registry*, CommandBuffer&, uint32_t)>;
using InitFunc = CallMe::Delegate<void(Registry*, CommandBuffer&)>;

#define DEPENDENCIES(...) __VA_ARGS__

#define REGISTER_SYSTEM(SystemType, Phase, Before, After, SortIndex)                                                    \
    inline static struct Reg_##SystemType                                                                               \
    {                                                                                                                   \
        Reg_##SystemType()                                                                                              \
        {                                                                                                               \
            SystemRegistry::Get().Add({ std::type_index(typeid(SystemType)),                                            \
                                        Phase,                                                                          \
                                        SortIndex,                                                                      \
                                        Before,                                                                         \
                                        After,                                                                          \
                                        []() -> std::unique_ptr<ISystem> { return std::make_unique<SystemType>(); } }); \
        }                                                                                                               \
    } instance_##SystemType;

enum class SystemPhase
{
    Simulation, // This runs in both sync and unsync
    Presentation,
};

struct SystemInfo
{
    std::type_index id;
    SystemPhase phase;
    uint32_t sortIndex;
    std::vector<std::type_index> before;
    std::vector<std::type_index> after;
    std::function<std::unique_ptr<ISystem>()> creator;
};

struct SystemMeta
{
    std::type_index id;
    SystemPhase phase;
    std::unique_ptr<ISystem> system;
};

class SystemRegistry
{
  public:
    static SystemRegistry& Get();

    void Init(Registry* registry, CommandBuffer& cmd);

    void Run(Registry* registry, CommandBuffer& cmd, SystemPhase phase);
    void RunSync(Registry* registry, CommandBuffer& globalCmd, uint32_t tick, SystemPhase phase);

    void Add(const SystemInfo& info);

  private:
    void Bake();

    std::vector<SystemInfo> infos;
    std::vector<SystemMeta> sortedSystems;
};