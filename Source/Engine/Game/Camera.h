#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

constexpr glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, 1.0f);

constexpr uint32_t PROJECTION_CHANGED = 1 << 0;
constexpr uint32_t VIEW_CHANGED = 1 << 1;

struct CameraTransform
{
    glm::quat rotation;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    uint32_t flags = PROJECTION_CHANGED | VIEW_CHANGED;

    CameraTransform& LookAt(const glm::vec3& target, const glm::vec3& up = WORLD_UP)
    {
        glm::mat4 view = glm::lookAt(position, target, up);
        rotation = glm::conjugate(glm::quat_cast(view));
        flags |= VIEW_CHANGED;
        return *this;
    }
};

struct CameraGlobalTransform
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 projectionView;
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
