#pragma once

#include <glm/ext/matrix_float4x4.hpp>

struct SharedConstant
{
    alignas(16) glm::mat4 model;
};