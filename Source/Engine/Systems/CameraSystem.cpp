#include "CameraSystem.h"
#include "Game/Camera.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Systems/SystemRegistry.h"
#include "Systems/TransformSystem.h"
#include <glm/fwd.hpp>

REGISTER_SYSTEM(CameraSystem, SystemPhase::Presentation, DEPENDENCIES({}), DEPENDENCIES({ typeid(TransformSystem) }), 100);

CameraSystem::CameraSystem()
{
    SystemRegistry& sreg = SystemRegistry::Get();
    sreg.BindFunc<CameraSystem, &CameraSystem::Run>(this);
    sreg.BindInitFunc<CameraSystem, &CameraSystem::Init>(this);
}

void CameraSystem::Init(Registry* registry, CommandBuffer& cmd)
{
    // I'm not going to support multiple cameras, that's why the cam is a resource
    registry->AddResource<Camera>();

    CameraTransform& camTransform = registry->AddResource<CameraTransform>(glm::quat(), glm::vec3(0.f, 45.f, -5.f));
    camTransform.LookAt(glm::vec3(0.0f));
    registry->AddResource<CameraGlobalTransform>();
}

void CameraSystem::Run(Registry* registry, CommandBuffer& cmd)
{
    RenderingSystem& rs = RenderingSystem::Get();
    entt::registry& reg = registry->Get();

    const Camera& cam = registry->GetResource<Camera>();
    CameraTransform& transform = registry->GetResource<CameraTransform>();
    CameraGlobalTransform& global = registry->GetResource<CameraGlobalTransform>();

    const float aspectRatio = RenderingSystem::Get().GetAspectRatio();

    if ((transform.flags & PROJECTION_CHANGED) != 0)
    {
        transform.flags &= ~PROJECTION_CHANGED;
        global.projection = Camera::CalculateProjection(cam, aspectRatio);
        global.projectionView = global.projection * global.view;
    }

    if ((transform.flags & VIEW_CHANGED) != 0)
    {
        transform.flags &= ~VIEW_CHANGED;

        transform.right = Camera::Right(transform);
        transform.forward = Camera::Forward(transform);
        transform.up = Camera::Up(transform);

        global.view = Camera::CalculateView(transform);
        global.projectionView = global.projection * global.view;
    }

    rs.UpdateCameraMatrix(global.projectionView);
}
