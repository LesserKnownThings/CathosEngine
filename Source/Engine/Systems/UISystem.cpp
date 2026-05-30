#include "UISystem.h"
#include "Components/Hierarchy.h"
#include "InputManager.h"
#include "LUA/LuaUnsynced.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/Font.h"
#include "Resources/LuaResources.h"
#include "Systems/SystemRegistry.h"
#include "UI/Button.h"
#include "UI/Canvas.h"
#include "UI/LayoutBox.h"
#include "UI/MainOverlay.h"
#include "UI/TextRenderer.h"
#include "UI/UIEvent.h"
#include "UI/UIMaterial.h"
#include "UI/UIMeshData.h"
#include "UI/UITransform.h"
#include "UI/UIVisibility.h"
#include <SDL3/SDL_mouse.h>
#include <entt/entity/fwd.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <sstream>

REGISTER_SYSTEM(UISystem, SystemPhase::Presentation, DEPENDENCIES({}), DEPENDENCIES({}), 0);

inline void ScaleUI(CanvasScaler& scaler, MainOverlay& overlay, Canvas& canvas, float width, float height)
{
    scaler.windowSize = { width, height };

    float scaleX = width / scaler.referenceResolution.x;
    float scaleY = height / scaler.referenceResolution.y;

    float logX = std::log2(scaleX);
    float logY = std::log2(scaleY);
    float logWeightedAverage = logX + (logY - logX) * scaler.matchWidthOrHeight;
    float scaleFactor = std::pow(2.0f, logWeightedAverage);

    const float scaledWidth = width / scaleFactor;
    const float scaledHeight = height / scaleFactor;

    overlay.width = scaledWidth;
    overlay.height = scaledHeight;

    canvas.projection = glm::ortho(0.0f, scaledWidth, scaledHeight, 0.0f, -1000.0f, 1000.0f);
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

    registry->AddResource<UIEvent>();
    CanvasScaler& scaler = registry->AddResource<CanvasScaler>();
    MainOverlay& overlay = registry->AddResource<MainOverlay>();
    Canvas& canvas = registry->AddResource<Canvas>();

    ScaleUI(scaler, overlay, canvas, width, height);

    windowResizedHandle = rs.onWindowResizeParam.subscribe(CallMe::fromMethod<&UISystem::HandleWindowResized>(this));
}

inline void ProcessUIVisibility(entt::registry& reg)
{
    auto view = reg.view<UIVisiblity>();

    // Process visibility mode first
    auto visiblityModeFunc = [&](UIVisiblity& visibility)
    {
        switch (visibility.mode)
        {
        case VisibilityMode::Visibile:
            visibility.isVisible = true;
            break;
        default:
            visibility.isVisible = false;
            break;
        }
    };
    view.each(visiblityModeFunc);

    const auto& parentPool = reg.storage<ChildOf>();

    auto hierarchyFunc = [&](entt::entity entity, UIVisiblity& visibility) -> void
    {
        if (parentPool.contains(entity))
        {
            const auto& parent = parentPool.get(entity);
            const UIVisiblity& parentVisibility = view.get<const UIVisiblity>(parent.entity);

            visibility.isVisible = parentVisibility.isVisible;
        }
    };
    view.each(hierarchyFunc);
}

inline void ComputeSingleUITransform(entt::registry& reg, entt::entity entity, entt::entity parentEntity, const MainOverlay& overlay)
{
    auto& renderTransform = reg.get<UIRenderTransform>(entity);
    const auto& transform = reg.get<UITransform>(entity);

    if (auto* visibility = reg.try_get<UIVisiblity>(entity))
    {
        if (!visibility->isVisible && visibility->mode == VisibilityMode::Collapsed)
        {
            renderTransform.position = glm::vec2(0.0f);
            renderTransform.size = glm::vec2(0.0f);
            return;
        }
    }

    glm::vec2 originPosition = glm::vec2(0.0f);
    glm::vec2 originSize = glm::vec2(overlay.width, overlay.height);

    if (parentEntity != entt::null)
    {
        const auto& parentTransform = reg.get<UIRenderTransform>(parentEntity);
        originPosition = parentTransform.position;
        originSize = parentTransform.size;
    }

    glm::vec2 anchorMin = transform.anchorMin;
    glm::vec2 anchorMax = transform.anchorMax;

    anchorMin.x = originPosition.x + (originSize.x * anchorMin.x);
    anchorMin.y = originPosition.y + (originSize.y * anchorMin.y);
    anchorMax.x = originPosition.x + (originSize.x * anchorMax.x);
    anchorMax.y = originPosition.y + (originSize.y * anchorMax.y);

    glm::vec2 anchorRectSize = anchorMax - anchorMin;
    glm::vec2 pivotOffset = transform.size * transform.pivot;

    bool stretchX = transform.anchorMin.x != transform.anchorMax.x;
    bool stretchY = transform.anchorMin.y != transform.anchorMax.y;

    glm::vec2 finalSize = transform.size;
    glm::vec2 finalPosition = anchorMin + transform.position - pivotOffset;

    if (stretchX)
    {
        finalSize.x = anchorRectSize.x - transform.position.x - transform.size.x;
        finalPosition.x = transform.position.x + anchorMin.x;
    }
    if (stretchY)
    {
        finalSize.y = anchorRectSize.y - transform.position.y - transform.size.y;
        finalPosition.y = transform.position.y + anchorMin.y;
    }

    renderTransform.position = finalPosition;
    renderTransform.size = finalSize;
}

inline void ApplyLayoutBoxToChildren(entt::registry& reg, entt::entity parent)
{
    const auto* childrenComp = reg.try_get<Children>(parent);
    if (!childrenComp || childrenComp->children.empty())
        return;

    const auto& renderTransform = reg.get<UIRenderTransform>(parent);

    if (const auto* vBox = reg.try_get<VBox>(parent))
    {
        const int32_t count = childrenComp->children.size();
        const float fCount = static_cast<float>(count);
        const float offsetSize = vBox->offset.z + vBox->offset.w;
        const float spacingSize = (vBox->spacing * (fCount - 1.0f)) + offsetSize;
        const float equalVSize = (renderTransform.size.y - spacingSize) / fCount;

        float totalChildrenHeight = 0.0f;
        if (vBox->controlVSize)
        {
            totalChildrenHeight = renderTransform.size.y - offsetSize;
        }
        else
        {
            for (entt::entity child : childrenComp->children)
            {
                totalChildrenHeight += reg.get<UIRenderTransform>(child).size.y;
            }
            totalChildrenHeight += vBox->spacing * (fCount - 1.0f);
        }

        float alignmentOffset = 0.0f;
        const float availableHeight = renderTransform.size.y - offsetSize;
        const float remainingSpace = availableHeight - totalChildrenHeight;

        switch (vBox->childStart)
        {
        case ChildStart::Middle:
            alignmentOffset = remainingSpace * 0.5f;
            break;
        case ChildStart::End:
            alignmentOffset = remainingSpace;
            break;
        default:
            alignmentOffset = 0.0f;
            break;
        }

        float cursorY = renderTransform.position.y + vBox->offset.z + alignmentOffset;

        for (entt::entity child : childrenComp->children)
        {
            auto& childTransform = reg.get<UIRenderTransform>(child);
            if (vBox->controlVSize)
                childTransform.size.y = equalVSize;
            if (vBox->controlHSize)
                childTransform.size.x = renderTransform.size.x - (vBox->offset.x + vBox->offset.y);

            childTransform.position.x = renderTransform.position.x + vBox->offset.x;
            childTransform.position.y = cursorY;

            cursorY += childTransform.size.y + vBox->spacing;
        }
    }

    if (const auto* hBox = reg.try_get<HBox>(parent))
    {
        const int32_t count = childrenComp->children.size();
        const float fCount = static_cast<float>(count);
        const float offsetSize = hBox->offset.x + hBox->offset.y;
        const float spacingSize = (hBox->spacing * (fCount - 1.0f)) + offsetSize;
        const float equalHSize = (renderTransform.size.x - spacingSize) / fCount;

        float totalChildrenWidth = 0.0f;
        if (hBox->controlHSize)
        {
            totalChildrenWidth = renderTransform.size.x - offsetSize;
        }
        else
        {
            for (entt::entity child : childrenComp->children)
            {
                totalChildrenWidth += reg.get<UIRenderTransform>(child).size.x;
            }
            totalChildrenWidth += hBox->spacing * (fCount - 1.0f);
        }

        float alignmentOffset = 0.0f;
        const float availableWidth = renderTransform.size.x - offsetSize;
        const float remainingSpace = availableWidth - totalChildrenWidth;

        switch (hBox->childStart)
        {
        case ChildStart::Middle:
            alignmentOffset = remainingSpace * 0.5f;
            break;
        case ChildStart::End:
            alignmentOffset = remainingSpace;
            break;
        default:
            alignmentOffset = 0.0f;
            break;
        }

        float cursorX = renderTransform.position.x + hBox->offset.y + alignmentOffset;

        for (entt::entity child : childrenComp->children)
        {
            auto& childTransform = reg.get<UIRenderTransform>(child);
            if (hBox->controlHSize)
                childTransform.size.x = equalHSize;
            if (hBox->controlVSize)
                childTransform.size.y = renderTransform.size.y - (hBox->offset.z + hBox->offset.w);

            childTransform.position.x = cursorX;
            childTransform.position.y = renderTransform.position.y + hBox->offset.z;

            cursorX += childTransform.size.x + hBox->spacing;
        }
    }
}

inline void UpdateLayoutAndTransforms(entt::registry& reg)
{
    MainOverlay& overlay = reg.ctx().get<MainOverlay>();
    auto rootView = reg.view<UIRenderTransform>(entt::exclude<ChildOf>);

    auto resolveHierarchy = [&](auto& self, entt::entity entity, entt::entity parentEntity, int32_t parentZ) -> void
    {
        auto& currentOrder = reg.get<UIRenderTransform>(entity);
        currentOrder.renderOrder = parentZ + 1;

        if (reg.all_of<Children>(entity))
        {
            const auto& childrenComp = reg.get<Children>(entity);

            for (entt::entity child : childrenComp.children)
            {
                ComputeSingleUITransform(reg, child, entity, overlay);
            }

            ApplyLayoutBoxToChildren(reg, entity);

            for (entt::entity child : childrenComp.children)
            {
                self(self, child, entity, currentOrder.renderOrder);
            }
        }
    };

    for (entt::entity root : rootView)
    {
        ComputeSingleUITransform(reg, root, entt::null, overlay);

        auto& rootOrder = reg.get<UIRenderTransform>(root);
        if (auto* transform = reg.try_get<UITransform>(root))
        {
            rootOrder.renderOrder = transform->localZOrder;
        }

        resolveHierarchy(resolveHierarchy, root, entt::null, rootOrder.renderOrder);
    }

    reg.sort<UIRenderTransform>([](const UIRenderTransform& lhs, const UIRenderTransform& rhs)
                                { return lhs.renderOrder < rhs.renderOrder; });
}

inline void UpdateInteraction(entt::registry& reg)
{
    const CanvasScaler& scaler = reg.ctx().get<CanvasScaler>();
    UIEvent& event = reg.ctx().get<UIEvent>();
    const LuaState& lua = reg.ctx().get<LuaState>();

    InputManager& im = InputManager::Get();
    const glm::vec2& mousePos = im.GetMousePosition() * scaler.GetScreenToCanvasScale();

    auto& transformStorage = reg.storage<UIRenderTransform>();
    auto& styleStorage = reg.storage<UIEventStyle>();
    auto& buttonStorage = reg.storage<Button>();
    auto& materialStorage = reg.storage<UIMaterial>();

    entt::entity hovered = entt::null;

    auto view = reg.view<const UIRenderTransform, const UIEventStyle>();
    auto entities = std::vector<entt::entity>(view.begin(), view.end());
    for (int32_t i = entities.size() - 1; i >= 0; --i)
    {
        auto entity = entities[i];
        const UIRenderTransform& transform = transformStorage.get(entity);

        if (transform.Overlaps(mousePos))
        {
            hovered = entity;
            break;
        }
    }

    entt::entity oldHovered = event.hovered;
    entt::entity oldPressed = event.pressed;

    event.hovered = hovered;

    const bool mouseJustPressed = im.IsMouseButtonJustPressed(SDL_BUTTON_LEFT);
    const bool mouseIsHeldDown = im.IsMouseButtonDown(SDL_BUTTON_LEFT);
    const bool mouseJustReleased = im.IsMouseButtonJustReleased(SDL_BUTTON_LEFT);

    if (mouseJustPressed)
    {
        event.pressed = hovered;
    }
    else if (mouseJustReleased)
    {
        if (event.pressed != entt::null && event.pressed == hovered)
        {
            if (buttonStorage.contains(event.pressed))
            {
                auto& button = buttonStorage.get(event.pressed);
                button.onClick.publish();
                LuaUnsynced::DispatchClick(lua.unsyncedState, event.pressed);
            }
        }

        event.pressed = entt::null;
    }
    else if (!mouseIsHeldDown)
    {
        event.pressed = entt::null;
    }

    auto updateEntityVisual = [&](entt::entity entity)
    {
        if (entity == entt::null)
        {
            return;
        }

        if (!styleStorage.contains(entity) || !materialStorage.contains(entity))
            return;

        const UIEventStyle& style = styleStorage.get(entity);
        UIMaterial& material = materialStorage.get(entity);

        if (entity == event.pressed && entity == event.hovered)
        {
            material.color = style.press;
        }
        else if (entity == event.hovered)
        {
            material.color = style.hover;
        }
        else
        {
            material.color = style.normal;
        }
    };

    if (oldHovered != event.hovered)
    {
        updateEntityVisual(oldHovered);
        updateEntityVisual(event.hovered);
    }
    if (oldPressed != event.pressed)
    {
        updateEntityVisual(oldPressed);
        updateEntityVisual(event.pressed);
    }
}

inline std::vector<std::string> Split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

void UISystem::Run(Registry* registry, CommandBuffer& cmd)
{
    entt::registry& reg = registry->Get();

    // Since text rendering requires a text style component to render I'm making sure to add one if it's missing
    reg.view<const TextRenderer>(entt::exclude<TextStyle>).each([&](entt::entity entity, const TextRenderer& tr)
                                                                { reg.emplace<TextStyle>(entity); });

    // I've noticed that other engines count the words for better text placement, trying something similar
    auto textDetailsSetup = [&](entt::entity entity, const TextRenderer& tr) -> void
    {
        const Font& font = tr.font;
        TextRendererDetails& textDetails = reg.emplace<TextRendererDetails>(entity, Split(tr.text, ' '));
        textDetails.widths.reserve(textDetails.words.size());

        for (const std::string& word : textDetails.words)
        {
            float width = 0.0f;
            for (int32_t l = 0; l < word.size(); ++l)
            {
                const char letter = word[l];

                auto glyphIT = font.mappedGlyphs.find(letter);
                if (glyphIT == font.mappedGlyphs.end())
                    continue;

                width += glyphIT->second.advance * tr.fontSize;
            }
            textDetails.widths.push_back(width);
        }
    };
    reg.view<const TextRenderer>(entt::exclude<TextRendererDetails>).each(textDetailsSetup);

    // Adding missing UI components
    auto& renderTransformStorage = reg.storage<UIRenderTransform>();
    auto& visibilityStorage = reg.storage<UIVisiblity>();

    auto layoutFunc = [&](entt::entity entity, const UITransform& transform)
    {
        if (!renderTransformStorage.contains(entity))
        {
            renderTransformStorage.emplace(entity);
        }

        if (!visibilityStorage.contains(entity))
        {
            visibilityStorage.emplace(entity);
        }
    };
    reg.view<const UITransform>().each(layoutFunc);

    if (windowResized)
    {
        windowResized = false;

        CanvasScaler& scaler = registry->GetResource<CanvasScaler>();
        MainOverlay& overlay = registry->GetResource<MainOverlay>();
        Canvas& canvas = registry->GetResource<Canvas>();

        ScaleUI(scaler, overlay, canvas, cachedWidth, cachedHeight);
    }

    ProcessUIVisibility(reg);
    UpdateLayoutAndTransforms(reg);
    UpdateInteraction(reg);
}

void UISystem::HandleWindowResized(float width, float height)
{
    windowResized = true;
    cachedWidth = width;
    cachedHeight = height;
}
