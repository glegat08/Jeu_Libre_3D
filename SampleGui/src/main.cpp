#include <filesystem>
#include <iostream>
#include <cmath>
#include "Core/CameraComponent.h"

#include "VulkanCore.h"
#include "_GLFW.h"
#include "Core/ManagerImple.h"
#include "Core/LightComponent.h"
#include "KGR_ImGui.h"
#include "Core/Frenet.h"
#include "Core/Spline.h"
#include <glm/gtc/constants.hpp>
#include "Core/Mesh.h"
#include "Context.h"

//----------------------------------------------------------------
// WORK IN PROGRESS
//----------------------------------------------------------------

int main(int argc, char** argv)
{
    std::filesystem::path exePath = argv[0];
    std::filesystem::path projectRoot = exePath.parent_path()
        .parent_path()
        .parent_path()
        .parent_path()
        .parent_path();

    FileManager::SetGlobalFIlePath(projectRoot / "Ressources");
    STBManager::SetGlobalFIlePath(projectRoot / "Ressources");
    MeshLoader::SetGlobalFIlePath(projectRoot / "Ressources");
    TextureLoader::SetGlobalFIlePath(projectRoot / "Ressources");

    KGR::_GLFW::Window::Init();
    KGR::_GLFW::Window::AddHint(GLFW_CLIENT_API, GLFW_NO_API);
    KGR::_GLFW::Window::AddHint(GLFW_RESIZABLE, GLFW_TRUE);
    KGR::_GLFW::Window window;
    window.CreateMyWindow({ 1400, 900 }, "KGR Engine", nullptr, nullptr);

    KGR::_Vulkan::VulkanCore app{};
    KGR::_ImGui::ImGuiCore   imguiCore;

    app.initVulkan(&window.GetWindow());
    imguiCore.InitImGui(&app, &window);

    ImGuiIO& io = imguiCore.GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // --- Editor ---
    KGR::Editor::Context editor(imguiCore, app);
    editor.Init(1280, 720);

    // --- Lights ---
    auto lComp = LightComponent<LightData::Type::Directional>::Create({ 1,1,1 }, { 1,1,1 }, 10.0f);
    TransformComponent lTransform;
    lTransform.LookAtDir({ 1,-1,1 });

    auto lComp2 = LightComponent<LightData::Type::Point>::Create({ 0,0,1 }, { 1,1,1 }, 10.0f, 10.0f);

    auto lComp3 = LightComponent<LightData::Type::Spot>::Create({ 0,1,0 }, { 1,1,1 }, 100.0f, 10.0f, glm::radians(45.0f), 10.0f);
    TransformComponent lTransform3;
    lTransform3.SetPosition({ -5,1,0 });
    lTransform3.LookAtDir({ 1,-1,0 });

    TextureComponent baseTexture;
    baseTexture.SetSize(1);
    baseTexture.AddTexture(0, &TextureLoader::Load("Textures\\BaseTexture.png", &app));

    auto lastTime = std::chrono::high_resolution_clock::now();

    // ---------------------------------------------------------------------------
    // Frame loop
    // ---------------------------------------------------------------------------
    do
    {
        KGR::_GLFW::Window::PollEvent();

        auto  currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        imguiCore.UpdateCamera(deltaTime);

        // --- ImGui frame ---
        imguiCore.BeginFrame(KGR::_ImGui::ContextTarget::Engine);
        ImGuizmo::BeginFrame();
        {
            editor.Render();

            // --- Debug draw clipped to the viewport rect ---
            {
                glm::vec2 vpPos = editor.GetViewportPos();
                glm::vec2 vpSize = editor.GetViewportSize();

                ImDrawList* dl = ImGui::GetBackgroundDrawList();
                dl->PushClipRect(
                    ImVec2(vpPos.x, vpPos.y),
                    ImVec2(vpPos.x + vpSize.x, vpPos.y + vpSize.y),
                    true);

                dl->PopClipRect();
            }
        }
        imguiCore.EndFrame();


        // --- Render scene entities ---
        std::vector<std::vector<Texture*>> entityTextures;
        {
            Scene* activeScene = editor.GetActiveScene();
            if (activeScene)
            {
                auto view = activeScene->GetRegistry().GetAllComponentsView<MeshComponent, TransformComponent>();
                entityTextures.reserve(view.Size());
                for (auto e : view)
                {
                    auto& meshComp = activeScene->GetComponent<MeshComponent>(e);
                    auto& transformComp = activeScene->GetComponent<TransformComponent>(e);
                    if (meshComp.mesh)
                    {
                        entityTextures.emplace_back(
                            meshComp.mesh->GetSubMeshesCount(),
                            baseTexture.GetTexture(0));
                        app.RegisterRender(*meshComp.mesh, transformComp.GetFullTransform(), entityTextures.back());
                    }
                }
            }
        }

        app.RegisterCam(
            editor.GetCamTransform().GetFullTransform(),
            editor.GetCamera().GetView(),
            editor.GetCamera().GetProj());

        {
            LightData ld1 = lComp.ToData();
            ld1.dir = lTransform.GetLocalAxe<RotData::Dir::Forward>();
            app.RegisterLight(ld1);
        }
        {
            LightData ld2 = lComp2.ToData();
            app.RegisterLight(ld2);
        }
        {
            LightData ld3 = lComp3.ToData();
            ld3.pos = lTransform3.GetPosition();
            ld3.dir = lTransform3.GetLocalAxe<RotData::Dir::Forward>();
            app.RegisterLight(ld3);
        }

        app.Render(&window.GetWindow(), { 0.53f, 0.81f, 0.92f, 1.0f },
            imguiCore.GetDrawData(), &editor.GetOffscreen());

    } while (!window.ShouldClose());

    // --- Cleanup ---
    app.GetDevice().Get().waitIdle();
    editor.Destroy();
    imguiCore.Destroy();
    window.DestroyMyWindow();
    MeshLoader::UnloadAll();
    TextureLoader::UnloadAll();
    KGR::_GLFW::Window::Destroy();
}