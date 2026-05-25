#include "Factories.h"
#include "Components/Collider.h"
#include "Components/Navigation/PathRequest.h"
#include "Components/Transform.h"
#include "Components/Visibility.h"
#include "Game/Camera.h"
#include "Resources/MaterialMesh3D.h"
#include "Resources/Model.h"
#include "UI/LayoutBox.h"
#include "UI/NineSlice.h"
#include "UI/TextRenderer.h"
#include "UI/UIMaterial.h"
#include "UI/UITransform.h"

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

    // Physics
    RegisterComponentType<Collider>();

    // AI
    RegisterComponentType<PathRequest>();

    // UI
    RegisterComponentType<TextRenderer>();
    RegisterComponentType<TextStyle>();

    RegisterComponentType<UITransform>();
    RegisterComponentType<UIRenderTransform>();
    RegisterComponentType<UIRenderOrder>();
    RegisterComponentType<UIAnchor>();
    RegisterComponentType<UIPivot>();
    RegisterComponentType<ChildOf>();
    RegisterComponentType<Children>();

    RegisterComponentType<UIMaterial>();
    RegisterComponentType<NineSlice>();

    RegisterComponentType<HBox>();
    RegisterComponentType<VBox>();
    RegisterComponentType<GridBox>();
}