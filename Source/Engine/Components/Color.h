#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

namespace Color
{
constexpr glm::vec4 WHITE(1.0f);
constexpr glm::vec4 BLACK(glm::vec3(0.0f), 1.0f);
constexpr glm::vec4 YELLOW(1.0f, 1.0f, 0.f, 1.0f);
constexpr glm::vec4 BLUE(0.0f, 0.0f, 1.f, 1.0f);
}; // namespace Color