#include "Factories.h"
#include "Components/Transform.h"
#include "Components/Visibility.h"
#include "Game/Camera.h"
#include "Resources/MaterialMesh3D.h"
#include "Resources/Model.h"

std::unordered_map<uint32_t, Factories::EmplaceFn> Factories::componentFactory;

Factories::Factories()
{
    // Camera stuff
    RegisterComponentType<Camera>();
    RegisterComponentType<CameraTransform>();
    RegisterComponentType<CameraGlobalTransform>();

    // World stuff
    RegisterComponentType<LocalTransform>();
    RegisterComponentType<GlobalTransform>();
    RegisterComponentType<RenderTransform>();
    RegisterComponentType<Hierarchy>();

    RegisterComponentType<MaterialMesh3D>();
    RegisterComponentType<Mesh>();

    RegisterComponentType<Visible>();
}