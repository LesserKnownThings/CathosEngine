#pragma once

#include "Components/Transform.h"
#include "Factories.h"
#include "Registry.h"
#include <cstddef>
#include <cstdint>
#include <entt/core/fwd.hpp>
#include <entt/core/type_info.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entt.hpp>
#include <limits>
#include <typeindex>
#include <vector>

constexpr uint32_t INVALID_ENTITY = std::numeric_limits<uint32_t>::max();

struct CommandHeader
{
    std::type_index type;
    uint32_t size;
};

struct CreateEntityCommand
{
};

struct AddComponentCommand
{
    uint32_t tempId; // temporary entity used to create component
    entt::id_type typeId;
    uint32_t dataSize;
};

struct RemoveComponentCommand
{
    entt::id_type typeId;
    entt::entity entity;
};

struct DestroyEntityCommand
{
    entt::entity entity;
};

struct LinkCommand
{
    uint32_t child;
    uint32_t parent;
};

class CommandBuffer
{
  public:
    uint32_t CreateEntity()
    {
        uint32_t id = entityCounter++;
        CommandHeader header{ typeid(CreateEntityCommand), sizeof(uint32_t) };
        RecordRaw(&header, sizeof(CommandHeader));
        RecordRaw(&id, sizeof(uint32_t));
        return id;
    }

    template <typename T>
    void AddComponent(uint32_t entity, const T& component)
    {
        CommandHeader header{ typeid(AddComponentCommand), sizeof(AddComponentCommand) + sizeof(T) };
        RecordRaw(&header, sizeof(CommandHeader));

        AddComponentCommand cmd{ entity, entt::type_id<T>().hash(), (uint32_t)sizeof(T) };
        RecordRaw(&cmd, sizeof(AddComponentCommand));

        RecordRaw(&component, sizeof(T));
    }

    template <typename T>
    void RemoveComponent(entt::entity entity)
    {
        CommandHeader header{ typeid(RemoveComponentCommand), sizeof(RemoveComponentCommand) };
        RecordRaw(&header, sizeof(CommandHeader));

        RemoveComponentCommand cmd{ entity, entt::type_id<T>().hash, entity };
        RecordRaw(&cmd, sizeof(RemoveComponentCommand));
    }

    void Destroy(entt::entity entity)
    {
        CommandHeader header{ typeid(DestroyEntityCommand), sizeof(CommandHeader) };
        RecordRaw(&header, sizeof(CommandHeader));

        DestroyEntityCommand cmd{ entity };
        RecordRaw(&cmd, sizeof(DestroyEntityCommand));
    }

    template <typename T>
    T* Get(uint32_t entity)
    {
        size_t offset = 0;
        while (offset < buffer.size())
        {
            CommandHeader* header = (CommandHeader*)&buffer[offset];
            uint8_t* data = &buffer[offset + sizeof(CommandHeader)];

            if (header->type == typeid(AddComponentCommand))
            {
                auto& cmd = *reinterpret_cast<AddComponentCommand*>(data);

                if (cmd.tempId == entity && cmd.typeId == entt::type_id<T>().hash())
                {
                    return reinterpret_cast<T*>(data + sizeof(AddComponentCommand));
                }
            }
            offset += sizeof(CommandHeader) + header->size;
        }
        return nullptr;
    }

    void Link(uint32_t child, uint32_t parent)
    {
        CommandHeader header{ typeid(LinkCommand), sizeof(LinkCommand) };
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

            if (header->type == typeid(CreateEntityCommand))
            {
                auto& cmd = *reinterpret_cast<uint32_t*>(data);
                tempToReal[cmd] = reg.create();
            }
            else if (header->type == typeid(AddComponentCommand))
            {
                auto& cmd = *reinterpret_cast<AddComponentCommand*>(data);
                uint8_t* componentData = data + sizeof(AddComponentCommand);

                // Retrieve the real entity from our mapping table
                entt::entity realEntity = tempToReal[cmd.tempId];

                if (realEntity != entt::null)
                {
                    Factories::EmplaceByType(reg, realEntity, cmd.typeId, componentData);
                }
            }
            else if (header->type == typeid(RemoveComponentCommand))
            {
                RemoveComponentCommand& cmd = *reinterpret_cast<RemoveComponentCommand*>(data);

                auto* storage = reg.storage(cmd.typeId);

                if (storage && storage->contains(cmd.entity))
                {
                    storage->remove(cmd.entity);
                }
            }
            else if (header->type == typeid(LinkCommand))
            {
                LinkCommand& cmd = *reinterpret_cast<LinkCommand*>(data);

                entt::entity child = tempToReal[cmd.child];
                entt::entity parent = tempToReal[cmd.parent];
            }
            else if (header->type == typeid(DestroyEntityCommand))
            {
                auto& cmd = *reinterpret_cast<DestroyEntityCommand*>(data);
                reg.destroy(cmd.entity);
            }

            offset += sizeof(CommandHeader) + header->size;
        }

        buffer.clear();
    }

    uint32_t entityCounter = 0;
    std::vector<uint8_t> buffer;

    friend class World;
    friend class EditorWorld;
};