#include "SDLLayer.h"

#include "Debug/DebugSystem.h"
#include <SDL3/SDL.h>
#include <format>

constexpr std::string SDL_LOG = "SDL";

inline void CheckError()
{
    const char* error = SDL_GetError();
    if (*error != '\0')
    {
        LOG(SDL_LOG, Error, std::format("Error: {}", error));
    }

    SDL_ClearError();
}

bool SDLLayer::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
    {
        CheckError();
        return false;
    }
    return true;
}

void SDLLayer::Shutdown()
{
    SDL_Quit();
}
