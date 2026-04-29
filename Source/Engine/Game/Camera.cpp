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

void Camera::ExtractFrustum(Frustum& frustum, const glm::mat4& vp)
{
    // Left Plane
    frustum.planes[0].normal.x = vp[0][3] + vp[0][0];
    frustum.planes[0].normal.y = vp[1][3] + vp[1][0];
    frustum.planes[0].normal.z = vp[2][3] + vp[2][0];
    frustum.planes[0].distance = vp[3][3] + vp[3][0];

    // Right Plane
    frustum.planes[1].normal.x = vp[0][3] - vp[0][0];
    frustum.planes[1].normal.y = vp[1][3] - vp[1][0];
    frustum.planes[1].normal.z = vp[2][3] - vp[2][0];
    frustum.planes[1].distance = vp[3][3] - vp[3][0];

    // Bottom Plane
    frustum.planes[2].normal.x = vp[0][3] + vp[0][1];
    frustum.planes[2].normal.y = vp[1][3] + vp[1][1];
    frustum.planes[2].normal.z = vp[2][3] + vp[2][1];
    frustum.planes[2].distance = vp[3][3] + vp[3][1];

    // Top Plane
    frustum.planes[3].normal.x = vp[0][3] - vp[0][1];
    frustum.planes[3].normal.y = vp[1][3] - vp[1][1];
    frustum.planes[3].normal.z = vp[2][3] - vp[2][1];
    frustum.planes[3].distance = vp[3][3] - vp[3][1];

    // Near Plane
    frustum.planes[4].normal.x = vp[0][3] + vp[0][2];
    frustum.planes[4].normal.y = vp[1][3] + vp[1][2];
    frustum.planes[4].normal.z = vp[2][3] + vp[2][2];
    frustum.planes[4].distance = vp[3][3] + vp[3][2];

    // Far Plane
    frustum.planes[5].normal.x = vp[0][3] - vp[0][2];
    frustum.planes[5].normal.y = vp[1][3] - vp[1][2];
    frustum.planes[5].normal.z = vp[2][3] - vp[2][2];
    frustum.planes[5].distance = vp[3][3] - vp[3][2];

    for (int i = 0; i < 6; i++)
    {
        frustum.planes[i].Normalize();
    }
}