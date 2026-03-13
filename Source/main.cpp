#include "Engine.h"
#include <cstdint>

int32_t main(int argc, const char* argv[])
{
    Engine e;
    const bool val = e.Initialize(argc, argv);
    e.Shutdown();
    return val;
}