#pragma once

#include "Utilities/Hash.h"
#include <cstdint>
#include <functional>
#include <string>

struct AssetPath
{
    AssetPath() = delete;
    AssetPath(const std::string& inPath)
        : path(inPath), hash(Hash::FNV1Hash(inPath))
    {
    }

    AssetPath(const AssetPath& other)
        : path(other.path), hash(other.hash)
    {
    }

    AssetPath(AssetPath&& n) noexcept
        : path(n.path), hash(n.hash)
    {
    }

    AssetPath& operator=(const AssetPath& other)
    {
        hash = other.hash;
        path = other.path;
        return *this;
    }

    inline bool operator==(uint32_t otherHash) const
    {
        return hash == otherHash;
    }

    inline bool operator!=(uint32_t otherHash) const
    {
        return hash != otherHash;
    }

    inline bool operator==(const AssetPath& other) const
    {
        return hash == other.hash;
    }

    inline bool operator!=(const AssetPath& other) const
    {
        return !(other.hash == hash);
    }

    uint64_t GetHash() const { return hash; }
    const std::string& GetPath() const { return path; }
    bool IsValid() const { return hash != 0; }
    void Reset()
    {
        hash = 0;
    }

  private:
    uint64_t hash = 0;
    std::string path;

    friend struct std::hash<AssetPath>;
};

template <>
struct std::hash<AssetPath>
{
    uint64_t operator()(const AssetPath& s) const noexcept
    {
        return s.hash;
    }
};
