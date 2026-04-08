#pragma once
#define NOMINMAX

#include <glm/glm.hpp>
#include "Core/CameraComponent.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "ECS/Registry.h"

struct MeshComponent2;


struct FPSCameraState
{
    float yaw              = -90.0f; 
    float pitch            =   0.0f; 
    float mouseSensitivity =   0.1f;
    float moveSpeed        =   5.0f;
};


template<typename TRegistry, typename TPlayerTag>
void FPSCameraUpdate(
    FPSCameraState& state,
    TRegistry& registry,
    KGR::RenderWindow* window,
    float dt,
    glm::vec3& outFront)
{
    glm::vec2 mouseDelta = window->GetInputManager()->GetMouseDelta();
    state.yaw   += mouseDelta.x * state.mouseSensitivity;
    state.pitch += mouseDelta.y * state.mouseSensitivity;

    glm::vec3 front;
    front.x = cos(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    front.y = sin(glm::radians(state.pitch));
    front.z = sin(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    front = glm::normalize(front);
    outFront = front;

    glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 right     = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 playerPos = { 0,0,0 };
    {
        auto players = registry.template GetAllComponentsView<TPlayerTag, TransformComponent>();
        for (auto& e : players)
        {
            auto input = window->GetInputManager();
            TransformComponent& t = registry.template GetComponent<TransformComponent>(e);

            if (input->IsKeyDown(KGR::Key::Z)) t.Translate( flatFront * state.moveSpeed * dt);
            if (input->IsKeyDown(KGR::Key::S)) t.Translate(-flatFront * state.moveSpeed * dt);
            if (input->IsKeyDown(KGR::Key::Q)) t.Translate(-right     * state.moveSpeed * dt);
            if (input->IsKeyDown(KGR::Key::D)) t.Translate( right     * state.moveSpeed * dt);

            playerPos = t.GetPosition();
        }
    }

    auto cameras = registry.template GetAllComponentsView<CameraComponent, TransformComponent>();
    if (cameras.Size() != 1)
        throw std::runtime_error("FPSCamera: need exactly one camera entity");

    for (auto& cam : cameras)
    {
        TransformComponent& camTransform = registry.template GetComponent<TransformComponent>(cam);
        camTransform.SetPosition(playerPos);
        camTransform.LookAtDir(front);

        registry.template GetComponent<CameraComponent>(cam).UpdateCamera(camTransform.GetFullTransform());
        registry.template GetComponent<CameraComponent>(cam).SetAspect(window->GetSize().x, window->GetSize().y);
        window->RegisterCam(registry.template GetComponent<CameraComponent>(cam), camTransform);
    }
}
