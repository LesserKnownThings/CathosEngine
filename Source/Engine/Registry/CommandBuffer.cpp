#include "CommandBuffer.h"
#include "Components/Hierarchy.h"

void CommandBuffer::Destroy(entt::entity e)
{
    commands.emplace_back([=](entt::registry& reg)
                          { 
        if (!reg.valid(e)) return;

        if (auto* childOf = reg.try_get<ChildOf>(e)) 
        {
            if (auto* parentChildren = reg.try_get<Children>(childOf->entity)) 
            {
                auto& v = parentChildren->children;
                v.erase(std::remove(v.begin(), v.end(), e), v.end());
            }
        }

        auto destroyRecursive = [&](auto& self, entt::entity entity) -> void 
        {
            if (auto* childrenComp = reg.try_get<Children>(entity))
            {
                auto childrenCopy = childrenComp->children; 
                
                for (auto child : childrenCopy)
                {
                    if (reg.valid(child))
                    {
                        self(self, child);
                    }
                }
            }
            
            reg.destroy(entity);
        };

        destroyRecursive(destroyRecursive, e); });
}