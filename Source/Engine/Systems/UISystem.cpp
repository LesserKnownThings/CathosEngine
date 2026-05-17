#include "UISystem.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Systems/SystemRegistry.h"
#include "UI/Canvas.h"
#include "UI/UIMeshData.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

REGISTER_SYSTEM(UISystem, SystemPhase::Presentation, DEPENDENCIES({}), DEPENDENCIES({}), 0);

UISystem::UISystem()
{
    SystemRegistry& sreg = SystemRegistry::Get();
    sreg.BindInitFunc<UISystem, &UISystem::Init>(this);
}

void UISystem::Init(Registry* registry, CommandBuffer& cmd)
{
    RenderingSystem& rs = RenderingSystem::Get();

    std::vector<QuadVertex> vertices = std::vector<QuadVertex>(QuadVertices.begin(), QuadVertices.end());
    std::vector<uint32_t> indices = std::vector<uint32_t>(QuadIndices.begin(), QuadIndices.end());

    UIMeshData meshData{
        static_cast<uint32_t>(vertices.size()),
        static_cast<uint32_t>(indices.size()),
        vertices,
        indices,
    };
    UIMeshGPUData gpuData = rs.CreateMesh(meshData);
    // TODO add a shutdown function for systems for cleanup
    registry->AddResource<UIMeshGPUData>(gpuData);
    const float width = static_cast<float>(rs.GetContext().swapChainExtent.width);
    const float height = static_cast<float>(rs.GetContext().swapChainExtent.height);
    registry->AddResource(Canvas{ glm::ortho(0.0f, width, height, 0.0f, -1000.0f, 1000.0f) });
}