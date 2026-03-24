#pragma once

#include "Components/Transform.h"
#include "Factories.h"
#include "Registry.h"
#include <cstddef>
#include <cstdint>
#include <entt/entity/entity.hpp>
#include <entt/entt.hpp>
#include <limits>
#include <vector>

constexpr uint32_t INVALID_ENTITY = std::numeric_limits<uint32_t>::max();

struct CommandHeader
{
    uint32_t type;
    uint32_t size;
};

struct ComponentCommand
{
    uint32_t tempId;
    uint32_t typeId;
    uint32_t dataSize;
};

struct LinkCommand
{
    uint32_t child;
    uint32_t parent;
};

enum class CommandType : uint32_t
{
    Create,
    Link,
    Destroy,
    AddComponent
};

class CommandBuffer
{
  public:
    uint32_t CreateEntity()
    {
        uint32_t id = entityCounter++;
        CommandHeader header{ (uint32_t)CommandType::Create, sizeof(uint32_t) };
        RecordRaw(&header, sizeof(CommandHeader));
        RecordRaw(&id, sizeof(uint32_t));
        return id;
    }

    template <typename T>
    void AddComponent(uint32_t entity, const T& component)
    {
        CommandHeader header{ (uint32_t)CommandType::AddComponent, sizeof(ComponentCommand) + sizeof(T) };
        RecordRaw(&header, sizeof(CommandHeader));

        ComponentCommand comp{ entity, entt::type_id<T>().hash(), (uint32_t)sizeof(T) };
        RecordRaw(&comp, sizeof(ComponentCommand));

        RecordRaw(&component, sizeof(T));
    }

    template <typename T>
    T* Get(uint32_t entity)
    {
        size_t offset = 0;
        while (offset < buffer.size())
        {
            CommandHeader* header = (CommandHeader*)&buffer[offset];
            uint8_t* data = &buffer[offset + sizeof(CommandHeader)];

            if (header->type == (uint32_t)CommandType::AddComponent)
            {
                auto& cmd = *reinterpret_cast<ComponentCommand*>(data);

                if (cmd.tempId == entity && cmd.typeId == entt::type_id<T>().hash())
                {
                    return reinterpret_cast<T*>(data + sizeof(ComponentCommand));
                }
            }
            offset += sizeof(CommandHeader) + header->size;
        }
        return nullptr;
    }

    void Link(uint32_t child, uint32_t parent)
    {
        CommandHeader header{ static_cast<uint32_t>(CommandType::Link), sizeof(LinkCommand) };
        RecordRaw(&header, sizeof(CommandHeader));

        LinkCommand cmd{ child, parent };
        RecordRaw(&cmd, sizeof(LinkCommand));
    }

  private:
    void RecordRaw(const void* data, int32_t size)
    {
        if (size <= 0 || data == nullptr)
            return;

        const uint8_t* start = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), start, start + size);
    }

    void LinkEntities(entt::registry& registry, entt::entity child, entt::entity parent)
    {
        auto& childH = registry.get<Hierarchy>(child);
        auto& parentH = registry.get<Hierarchy>(parent);

        childH.parent = parent;

        if (parentH.firstChild != entt::null)
        {
            childH.nextSibling = parentH.firstChild;
        }

        parentH.firstChild = child;
    }

    void Execute(Registry* registry)
    {
        entt::registry& reg = registry->Get();
        std::vector<entt::entity> tempToReal(entityCounter);

        size_t offset = 0;
        while (offset < buffer.size())
        {
            CommandHeader* header = (CommandHeader*)&buffer[offset];
            uint8_t* data = &buffer[offset + sizeof(CommandHeader)];

            if (header->type == static_cast<uint32_t>(CommandType::Create))
            {
                auto& cmd = *reinterpret_cast<uint32_t*>(data);
                tempToReal[cmd] = reg.create();
            }
            else if (header->type == static_cast<uint32_t>(CommandType::AddComponent))
            {
                auto& cmd = *reinterpret_cast<ComponentCommand*>(data);
                uint8_t* componentData = data + sizeof(ComponentCommand);

                // Retrieve the real entity from our mapping table
                entt::entity realEntity = tempToReal[cmd.tempId];

                if (realEntity != entt::null)
                {
                    Factories::EmplaceByType(reg, realEntity, cmd.typeId, componentData);
                }
            }
            else if (header->type == static_cast<uint32_t>(CommandType::Link))
            {
                LinkCommand& cmd = *reinterpret_cast<LinkCommand*>(data);

                entt::entity child = tempToReal[cmd.child];
                entt::entity parent = tempToReal[cmd.parent];
            }

            offset += sizeof(CommandHeader) + header->size;
        }

        buffer.clear();
    }

    uint32_t entityCounter = 0;
    std::vector<uint8_t> buffer;

    friend class World;
    friend class MapWorld;
};