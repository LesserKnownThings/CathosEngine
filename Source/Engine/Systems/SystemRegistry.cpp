#include "SystemRegistry.h"

SystemRegistry& SystemRegistry::Get()
{
    static SystemRegistry instance;
    return instance;
}

void SystemRegistry::Init(Registry* registry, CommandBuffer& globalCmd)
{
    Bake();

    for (auto& info : infos)
    {
        if (info.creator)
        {
            info.creator();
            info.initFunc(registry, globalCmd);
        }
    }
}

void SystemRegistry::Bake()
{
    // TODO sort systems
}

void SystemRegistry::Run(Registry* registry, CommandBuffer& globalCmd, SystemPhase phase)
{
    for (auto& sys : infos)
    {
        if (sys.phase == phase)
        {
            sys.func(registry, globalCmd);
        }
    }
}

void SystemRegistry::RunSync(Registry* registry, CommandBuffer& globalCmd, uint32_t tick, SystemPhase phase)
{
    for (auto& sys : infos)
    {
        if (sys.phase == phase)
        {
            sys.syncFunc(registry, globalCmd, tick);
        }
    }
}