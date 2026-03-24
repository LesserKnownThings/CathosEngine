cmake_minimum_required(VERSION 3.2.8)
project(CathosEngine VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLOBAL_SOURCES "")
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/Source/*.cpp")
list(APPEND GLOBAL_SOURCES ${SOURCES})

if (MAP_EDITOR)
    file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/MapEditor/*.cpp")
    list(APPEND GLOBAL_SOURCES ${SOURCES})
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
###############

add_executable(CathosEngine ${GLOBAL_SOURCES})

target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/Source/Engine)
target_compile_definitions(CathosEngine PUBLIC VK_NO_PROTOTYPES)

option(MAP_EDITOR "Enable Map Editor" OFF)

if (MAP_EDITOR)
    target_compile_definitions(CathosEngine PUBLIC MAP_EDITOR=1 IMGUI_IMPL_VULKAN_USE_VOLK)
    target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/MapEditor)
    target_include_directories(CathosEngine PUBLIC ${CMAKE_SOURCE_DIR}/MapEditor/imgui)
else()
    target_compile_definitions(CathosEngine PUBLIC MAP_EDITOR=0)
endif()

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
)