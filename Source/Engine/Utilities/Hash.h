#pragma once

#include <cctype>
#include <cstdint>
#include <string>

class Hash
{
  public:
    static constexpr uint64_t FNV1Hash(const std::string& str)
    {
        constexpr uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;
        constexpr uint64_t FNV_PRIME = 0x100000001b3;

        uint64_t hash = FNV_OFFSET_BASIS;

        for (const char c : str)
        {
            hash *= FNV_PRIME;
            hash ^= static_cast<uint64_t>(std::tolower(c));
        }

        return hash;
    }
};