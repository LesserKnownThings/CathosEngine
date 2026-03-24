#pragma once

#include "Callme/CallMe.h"
#include <cstdint>
#include <functional>
#include <typeindex>
#include <vector>

class Registry;
class CommandBuffer;

using SystemFunc = CallMe::Delegate<void(Registry*, CommandBuffer&)>;
using SyncSystemFunc = CallMe::Delegate<void(Registry*, CommandBuffer&, uint32_t)>;
using InitFunc = CallMe::Delegate<void(Registry*, CommandBuffer&)>;

#define DEPENDENCIES(...) __VA_ARGS__

#define REGISTER_SYSTEM(SystemType, Phase, Before, After)                    \
    inline static struct Reg_##SystemType                                    \
    {                                                                        \
        Reg_##SystemType()                                                   \
        {                                                                    \
            SystemRegistry::Get().Add({ std::type_index(typeid(SystemType)), \
                                        Phase,                               \
                                        SystemFunc(),                        \
                                        SyncSystemFunc(),                    \
                                        InitFunc(),                          \
                                        Before,                              \
                                        After,                               \
                                        []() { SystemType{}; } });           \
        }                                                                    \
    } instance_##SystemType;

enum class SystemPhase
{
    Logic,
    Physics,
    Presentation,
};

struct SystemInfo
{
    std::type_index id;
    SystemPhase phase;
    SystemFunc func;
    SyncSystemFunc syncFunc;
    InitFunc initFunc;
    std::vector<std::type_index> before;
    std::vector<std::type_index> after;
    std::function<void()> creator;
};

class SystemRegistry
{
  public:
    static SystemRegistry& Get();

    void Init(Registry* registry, CommandBuffer& globalCmd);

    void Run(Registry* registry, CommandBuffer& globalCmd, SystemPhase phase);
    void RunSync(Registry* registry, CommandBuffer& globalCmd, uint32_t tick, SystemPhase phase);

    void Add(const SystemInfo& info)
    {
        infos.push_back(info);
    }

    template <typename T, auto M>
    void BindInitFunc(T* instance)
    {
        for (auto& system : infos)
        {
            if (system.id == typeid(T))
            {
                system.initFunc = InitFunc(instance, CallMe::tag<M>());
                break;
            }
        }
    }

    template <typename T, auto M>
    void BindFunc(T* instance)
    {
        for (auto& system : infos)
        {
            if (system.id == typeid(T))
            {
                system.func = SystemFunc(instance, CallMe::tag<M>());
                break;
            }
        }
    }

    template <typename T, auto M>
    void BindSyncFunc(T* instance)
    {
        for (auto& system : infos)
        {
            if (system.id == typeid(T))
            {
                system.syncFunc = SyncSystemFunc(instance, CallMe::tag<M>());
                break;
            }
        }
    }

  private:
    void Bake();

    std::vector<SystemInfo> infos;
};