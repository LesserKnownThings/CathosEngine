#pragma once

#include "VkData.h"

#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

struct Descriptor
{
    VkDescriptorSet set;
    VkDescriptorSetLayout layout;
};

struct TupleHash
{
    template <typename T1, typename T2>
    std::size_t operator()(const std::tuple<T1, T2>& t) const
    {
        std::size_t h1 = std::hash<T1>{}(std::get<0>(t));
        std::size_t h2 = std::hash<T2>{}(std::get<1>(t));
        // Combine the two hashes
        return h1 ^ (h2 << 1);
    }
};

struct DescriptorRegistry
{
    std::unordered_map<uint32_t, Descriptor> descriptors;
    std::unordered_map<std::tuple<int32_t, int32_t>, AllocatedBuffer, TupleHash> mappedBuffers;

    bool GetDescriptor(uint32_t set, Descriptor& outDescriptor) const
    {
        auto found = descriptors.find(set);
        if (found != nullptr)
        {
            outDescriptor = found->second;
            return true;
        }

        return false;
    }
};