#pragma once

#include <cstdint>
#include <vector>

template <typename T>
struct AssetHandle
{
    uint32_t index = 0;
    uint32_t generation = 0;
};

template <typename T>
struct AssetEntry
{
    T asset;
    uint32_t generation = 1;
    bool alive = true;
};

//**
// This system does not prevent asset duplicates
// TODO either implement that, or make sure no asset is loaded twice
//  */

template <typename T>
struct Assets
{
  private:
    using Handle = AssetHandle<T>;

    Handle Add(T&& asset)
    {
        uint32_t index;
        if (!freeIndices.empty())
        {
            index = freeIndices.back();
            freeIndices.pop_back();

            auto& entry = entries[index];
            entry.asset = std::move(asset);
            entry.generation++;
            entry.alive = true;
        }
        else
        {
            index = static_cast<uint32_t>(entries.size());
            entries.push_back({ std::move(asset), 1, true });
        }

        return { index, entries[index].generation };
    }

    T* Get(const Handle& handle)
    {
        if (!IsValid(handle))
            return nullptr;
        return &entries[handle.index].asset;
    }

    const T* Get(const Handle& handle) const
    {
        if (!IsValid(handle))
            return nullptr;
        return &entries[handle.index].asset;
    }

    void Remove(const Handle& handle)
    {
        if (!IsValid(handle))
            return;

        auto& entry = entries[handle.index];
        entry.alive = false;
        entry.generation++;
        freeIndices.push_back(handle.index);
    }

    bool IsValid(const Handle& handle) const
    {
        if (handle.index >= entries.size())
            return false;
        const auto& entry = entries[handle.index];
        return entry.alive && entry.generation == handle.generation;
    }

    void Clear()
    {
        entries.clear();
        freeIndices.clear();
    }

  private:
    std::vector<AssetEntry<T>> entries;
    std::vector<uint32_t> freeIndices;

    friend struct AssetServer;
};