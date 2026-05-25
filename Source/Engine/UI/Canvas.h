#pragma once

#include <glm/ext/matrix_float4x4.hpp>

struct Canvas
{
    glm::mat4 projection;
};

struct CanvasScaler
{
    glm::vec2 windowSize;
    glm::vec2 referenceResolution = { 1920.0f, 1080.0f };
    float matchWidthOrHeight = 0.5f;

    glm::vec2 GetScreenToCanvasScale() const
    {
        return {
            referenceResolution.x / windowSize.x,
            referenceResolution.y / windowSize.y,
        };
    }
};