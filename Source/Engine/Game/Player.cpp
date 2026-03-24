#include "Player.h"
#include "Callme/CallMe.h"
#include "Debug/DebugSystem.h"
#include "InputManager.h"
#include "Math/TransformMath.hpp"
#include "Netcode/Message.h"
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <string>
#include "Registry/Registry.h"

#include "Game/Camera.h"

constexpr std::string PLAYER_LOG = "Player";

Player::Player(Registry* inRegistry)
    : registry(inRegistry)
{
    InputManager& im = InputManager::Get();
    static auto mouseClickHandle = im.onMouseClicked.subscribe(CallMe::fromMethod<&Player::HandleMouseClick>(this));
    static auto mouseReleaseHandle = im.onMouseReleased.subscribe(CallMe::fromMethod<&Player::HandleMouseRelease>(this));
    static auto mouseMoveHandle = im.onMouseMoved.subscribe(CallMe::fromMethod<&Player::HandleMouseMove>(this));

    entt::registry& r = registry->Get();

    auto camEntity = r.create();
    r.emplace<Camera>(camEntity);
    r.emplace<CameraTransform>(camEntity, CameraTransform{ .position = glm::vec3(0.f, 30.f, -5.f) }.LookAt(glm::vec3(0.0f)));
    r.emplace<CameraGlobalTransform>(camEntity);
}

void Player::OnReceiveMessage(uint8_t* data)
{
    Message msg = MessagePacker::UnpackMessage(data);

    switch (msg.type)
    {
    // Chat is not a command so it can be processed directly
    case MessageType::Chat:
    {
        ChatMsg& chat = msg.GetChat();
        LOG(PLAYER_LOG, Log, chat.message);
    }

    default:
        pendingCommands.push_back(msg);
        break;
    }
}

void Player::Run(uint32_t tick)
{
    CameraMovement();
}

void Player::RunSim(uint32_t tick)
{
}

void Player::HandleMouseClick(uint32_t button)
{
    if (button == 1)
    {
        isMouseDown = true;
    }
    else if (button == 2)
    {
        rotateCamera = true;
        InputManager::Get().HideMouse();
    }
}

void Player::HandleMouseRelease(uint32_t button)
{
    if (button == 1)
    {
        isMouseDown = false;
    }
    else if (button == 2)
    {
        rotateCamera = false;
        InputManager::Get().ShowMouse();
    }
}

void Player::HandleMouseMove(const glm::vec2& pos)
{
    if (isMouseDown)
    {
    }
}

void Player::CameraMovement()
{
    auto view = registry->Get().view<CameraTransform>();

    InputManager& im = InputManager::Get();
    const glm::vec2& movementAxis = im.GetMovementAxis();
    const glm::vec2& mouseDelta = im.GetMouseDelta();
    const bool localRotateCamera = rotateCamera;

    constexpr float CAMERA_MOVEMENT_SPEED = 30.0f;
    constexpr float CAMERA_ROTATION_SPEED = 60.0f;

    auto func = [&movementAxis, &localRotateCamera, &mouseDelta](CameraTransform& transform)
    {
        if (glm::length(movementAxis) > 0.0f)
        {
            const glm::vec3 rightMovement = movementAxis.x * transform.right;
            glm::vec3 forward = movementAxis.y * transform.forward;
            // forward.y = 0.0f;

            const glm::vec3 final = glm::normalize(rightMovement + forward);

            transform.position += final * CAMERA_MOVEMENT_SPEED * TransformMath::NON_SIM_DT;

            transform.flags |= VIEW_CHANGED;
        }

        if (localRotateCamera)
        {
            float yaw = glm::radians(mouseDelta.x * CAMERA_ROTATION_SPEED * TransformMath::NON_SIM_DT);
            float pitch = glm::radians(mouseDelta.y * CAMERA_ROTATION_SPEED * TransformMath::NON_SIM_DT);

            glm::quat qPitch = glm::angleAxis(pitch, transform.right);
            glm::quat qYaw = glm::angleAxis(yaw, WORLD_UP);

            transform.rotation = qYaw * qPitch * transform.rotation;

            transform.flags |= VIEW_CHANGED;
        }
    };
    view.each(func);
}
