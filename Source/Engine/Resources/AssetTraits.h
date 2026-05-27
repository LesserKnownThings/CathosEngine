#pragma once

#include <cstdint>
#include <fstream>

struct FontAssetHeader;

template <typename T>
struct AssetTraits;

template <>
struct AssetTraits<FontAssetHeader>
{
    static constexpr uint32_t id = 0;
};

template <typename T>
inline void WriteAssetHeader(std::ofstream& out, T tag)
{
    const uint32_t n = AssetTraits<T>::id;
    out.write(reinterpret_cast<const char*>(&n), sizeof(uint32_t));
}