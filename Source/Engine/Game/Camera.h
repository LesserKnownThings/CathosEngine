#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

constexpr glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, 1.0f);

struct CameraTransform
{
    glm::quat rotation;
    glm::vec3 position;
    glm::vec3 scale;
};

struct Camera
{
    float fieldOfView = 60.0f;
    float nearView = 0.1f;
    float farView = 1000.0f;

    static glm::mat4 CalculateProjection(const Camera& camera, float aspectRatio);
    static glm::mat4 CalculateView(const CameraTransform& transform);

    static glm::vec3 Forward(const CameraTransform& transform);
    static glm::vec3 Right(const CameraTransform& transform);
    static glm::vec3 Up(const CameraTransform& transform);
};
