cmake_minimum_required(VERSION 3.2.8)
project(CathosEngine VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLOBAL_SOURCES "")
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/Source/*.cpp")
list(APPEND GLOBAL_SOURCES ${SOURCES})

if (EDITOR)
    file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/Editor/*.cpp")
    list(APPEND GLOBAL_SOURCES ${SOURCES})

    find_package(msdfgen CONFIG REQUIRED)
endif()

# THIRD PARTIES
add_compile_definitions(
    GLM_FORCE_LEFT_HANDED
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    VK_NO_PROTOTYPES)

find_package(volk CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Tracy REQUIRED)
find_package(FreeType REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED) # Not sure if I'm gonna use json??

include(FindLua)
find_package(Lua REQUIRED)
###############

add_executable(CathosEngine ${GLOBAL_SOURCES})

target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/Source/Engine)
target_compile_definitions(CathosEngine PUBLIC VK_NO_PROTOTYPES)

option(EDITOR "Enable Editor" OFF)

if (EDITOR)
    target_compile_definitions(CathosEngine PUBLIC EDITOR=1 IMGUI_IMPL_VULKAN_USE_VOLK)
    target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/Editor)
    target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/Editor/imgui)
    target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/Editor/msdf-atlas-gen)
    target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/Editor/msdfgen)

    target_link_libraries(CathosEngine
    PRIVATE
    ${CMAKE_SOURCE_DIR}/External/libmsdf-atlas-gen.so
    msdfgen::msdfgen
    )
else()
    target_compile_definitions(CathosEngine PUBLIC EDITOR=0)
endif()

target_link_libraries(CathosEngine PRIVATE ${LUA_LIBRARIES})
target_include_directories(CathosEngine PRIVATE ${LUA_INCLUDE_DIR})

target_link_libraries(CathosEngine
PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    volk::volk
    EnTT::EnTT
    glm::glm
    GameNetworkingSockets::GameNetworkingSockets
    assimp::assimp
    Tracy::TracyClient
    Freetype::Freetype
    nlohmann_json::nlohmann_json
)
