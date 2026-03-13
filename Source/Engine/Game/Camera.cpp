#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

glm::mat4 Camera::CalculateProjection(const Camera& camera, float aspectRatio)
{
    return glm::perspective(glm::radians(camera.fieldOfView), aspectRatio, camera.nearView, camera.farView);
}

glm::mat4 Camera::CalculateView(const CameraTransform& transform)
{
    return glm::lookAtLH(transform.position, transform.position + Forward(transform), Up(transform));
}

glm::vec3 Camera::Forward(const CameraTransform& transform)
{
    return glm::normalize(transform.rotation * WORLD_FORWARD);
}

glm::vec3 Camera::Right(const CameraTransform& transform)
{
    return glm::normalize(transform.rotation * WORLD_RIGHT);
}

glm::vec3 Camera::Up(const CameraTransform& transform)
{
    return glm::normalize(transform.rotation * WORLD_UP);
}