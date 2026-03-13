#pragma once

#include <cstdint>

struct IResourceCache
{
  public:
    virtual ~IResourceCache() = default;
    virtual void PurgeUnused() = 0;
    virtual uint32_t Count() const = 0;
};