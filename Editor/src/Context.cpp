#include "../../Editor/include/Context.h"
#include "Math/CollisionComponent.h"

namespace KGR
{
    namespace Editor
    {
        Context::Context(KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& vulkan)
            : m_imGui(imgui), m_vulkan(vulkan)
        {
        }

        void Context::Init(uint32_t viewportWidth, uint32_t viewportHeight)
        {
            RegisterClone();
            RegisterInspector();
            RegisterSerializer();

            m_toolbar = std::make_unique<Toolbar>(&m_editorScene);
            m_hierarchy = std::make_unique<HierarchyPanel>(&m_editorScene);
            m_inspector = std::make_unique<InspectorPanel>(this, &m_editorScene);
            m_viewport = std::make_unique<Viewport>(this, m_imGui, m_vulkan);
            m_menuBar = std::make_unique<MenuBar>(
                [this](const std::filesystem::path& p) { SaveScene(p); },
                [this](const std::filesystem::path& p) { LoadScene(p); });

            m_lastActiveScene = m_toolbar->GetActiveScene();

            m_camera = CameraComponent::Create(
                45.0f,
                static_cast<float>(viewportWidth),
                static_cast<float>(viewportHeight),
                0.01f, 1000.0f,
                CameraComponent::Type::Perspective);

            m_cameraTransform.SetPosition({ 0.0f, 5.0f, 5.0f });
            m_cameraTransform.LookAt({ 0.0f, 0.0f, 0.0f });
            m_camera.UpdateCamera(m_cameraTransform.GetFullTransform());
            m_imGui.SetCamera(&m_camera, &m_cameraTransform);

            VkDevice vkDevice = *m_vulkan.GetDevice().Get();
            VkPhysicalDevice vkPhysicDevice = *m_vulkan.GetPhysicalDevice().Get();
            VkDescriptorPool vkPool = *m_vulkan.GetDescriptorPool().Get();
            VkFormat colorFormat = static_cast<VkFormat>(m_vulkan.GetSwapChain().GetFormat().format);

            m_offscreen.Create(vkDevice, vkPhysicDevice, vkPool, viewportWidth, viewportHeight, colorFormat);
            m_viewport->SetSceneDescriptor(m_offscreen.GetDescriptorSet());

            Layer::ApplyStyle();
        }

        void Context::Render()
        {
            m_menuBar->Render();

            HandleUndoRedoShortcuts();
            HandleResize();

            Layer::BeginDockspace();

            m_toolbar->Render();

            Scene* activeScene = m_toolbar->GetActiveScene();
            if (m_lastActiveScene != activeScene)
            {
                m_undoManager.Clear();
                m_lastActiveScene = activeScene;
            }

            m_hierarchy->SetScene(activeScene);
            m_inspector->SetScene(activeScene);

            m_hierarchy->Render(m_picker);
            m_inspector->Render(m_picker.GetFirst());
            m_viewport->Render(m_picker, activeScene, &m_camera);

            Layer::EndDockspace();
        }

        void Context::HandleResize()
        {
            if (!m_viewport->WasResizedThisFrame())
                return;

            glm::vec2 newSize = m_viewport->GetSize();

            m_vulkan.GetDevice().Get().waitIdle();

            VkDevice vkDevice = *m_vulkan.GetDevice().Get();
            VkPhysicalDevice vkPhysicDevice = *m_vulkan.GetPhysicalDevice().Get();
            VkDescriptorPool vkPool = *m_vulkan.GetDescriptorPool().Get();

            m_offscreen.Resize(vkDevice, vkPhysicDevice, vkPool,
                static_cast<uint32_t>(newSize.x),
                static_cast<uint32_t>(newSize.y));

            m_viewport->SetSceneDescriptor(m_offscreen.GetDescriptorSet());

            m_camera = CameraComponent::Create(
                45.0f, newSize.x, newSize.y,
                0.01f, 1000.0f,
                CameraComponent::Type::Perspective);

            m_camera.UpdateCamera(m_cameraTransform.GetFullTransform());
            m_imGui.SetCamera(&m_camera, &m_cameraTransform);
        }

        void Context::Destroy()
        {
            VkDevice vkDevice = *m_vulkan.GetDevice().Get();
            m_offscreen.Destroy(vkDevice);
        }

        void Context::SaveScene(const std::filesystem::path& path)
        {
            Serializer::Save(m_editorScene, path);
        }

        void Context::LoadScene(const std::filesystem::path& path)
        {
            m_vulkan.GetDevice().Get().waitIdle();

            auto& reg = m_editorScene.GetRegistry();
            for (SceneEntity e : reg.GetAllEntities())
            {
                if (!reg.HasComponent<MeshComponent>(e))
                    continue;

                const auto& mc = reg.GetComponent<MeshComponent>(e);
                if (!mc.sourcePath.empty())
                    MeshLoader::Unload(mc.sourcePath);
            }

            m_editorScene = Scene{};
            RegisterClone();
            RegisterInspector();
            RegisterSerializer();

            m_picker.ClearSelection();
            m_undoManager.Clear();

            Serializer::Load(m_editorScene, path);
            m_lastActiveScene = m_toolbar ? m_toolbar->GetActiveScene() : &m_editorScene;
        }

        void Context::HandleUndoRedoShortcuts()
        {
            ImGuiIO& io = ImGui::GetIO();
            if (!io.KeyCtrl || io.WantTextInput)
                return;

            const bool undoKey = ImGui::IsKeyPressed(ImGuiKey_Z, false);

            if (undoKey)
            {
                if (io.KeyShift)
                    m_undoManager.Redo();
                else
                    m_undoManager.Undo();
                return;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Y, false))
                m_undoManager.Redo();
        }

        Scene* Context::GetActiveScene() const 
        { 
            return m_toolbar->GetActiveScene();
        }

        SceneEntity Context::GetSelectedEntity() const 
        { 
            return m_picker.GetFirst();
        }

        CameraComponent& Context::GetCamera() 
        { 
            return m_camera; 
        }

        TransformComponent& Context::GetCamTransform() 
        {
            return m_cameraTransform;
        }

        Offscreen& Context::GetOffscreen() 
        {
            return m_offscreen;
        }

        UndoManager& Context::GetUndoManager()
        {
            return m_undoManager;
        }

        glm::vec2 Context::GetViewportPos() const
        { 
            return m_viewport->GetPos();
        }

        glm::vec2 Context::GetViewportSize() const 
        { 
            return m_viewport->GetSize();
        }

        void Context::RegisterClone()
        {
            m_editorScene.RegisterClone<NameComponent>();
            m_editorScene.RegisterClone<HierarchyComponent>();
            m_editorScene.RegisterClone<TransformComponent>();
            m_editorScene.RegisterClone<MeshComponent>();
            m_editorScene.RegisterClone<CollisionComp>();
        }

        void Context::RegisterInspector()
        {
            m_editorScene.RegisterInspector<MeshComponent>("Mesh",
                [this](SceneEntity e, SceneRegistry& registry)
                {
                    if (!registry.HasComponent<MeshComponent>(e))
                    {
                        if (ImGui::Button("Add Mesh Component"))
                            registry.AddComponent<MeshComponent>(e);

                        return;
                    }

                    auto& mc = registry.GetComponent<MeshComponent>(e);

                    const std::string label = mc.sourcePath.empty() ? "(none)"
                        : std::filesystem::path(mc.sourcePath).filename().string();

                    ImGui::LabelText("File", "%s", label.c_str());

                    if (ImGui::Button("Load mesh..."))
                    {
                        std::string newPath = KGR::_ImGui::ImGuiCore::OpenFile();
                        if (!newPath.empty())
                        {
                            m_vulkan.GetDevice().Get().waitIdle();
                            if (!mc.sourcePath.empty())
                                MeshLoader::Unload(mc.sourcePath);

                            if (registry.HasComponent<CollisionComp>(e))
                            {
                                auto& cc = registry.GetComponent<CollisionComp>(e);
                                if (cc.collider)
                                    ColliderManager::Unload(mc.sourcePath);

                                registry.RemoveComponent<CollisionComp>(e);
                            }

                            mc.sourcePath = newPath;
                            mc.mesh = &MeshLoader::Load(newPath, &m_vulkan);
                        }
                    }

                    ImGui::SameLine();

                    if (mc.mesh && ImGui::Button("Clear"))
                    {
                        m_vulkan.GetDevice().Get().waitIdle();

                        if (registry.HasComponent<CollisionComp>(e))
                        {
                            auto& cc = registry.GetComponent<CollisionComp>(e);
                            if (cc.collider)
                                ColliderManager::Unload(mc.sourcePath);

                            registry.RemoveComponent<CollisionComp>(e);
                        }

                        MeshLoader::Unload(mc.sourcePath);
                        mc.mesh = nullptr;
                        mc.sourcePath.clear();
                    }
                });

            m_editorScene.RegisterInspector<CollisionComp>("Collider",
                [this](SceneEntity e, SceneRegistry& registry)
                {
                    if (!registry.HasComponent<CollisionComp>(e))
                    {
                        bool hasMesh = registry.HasComponent<MeshComponent>(e)
                            && registry.GetComponent<MeshComponent>(e).mesh != nullptr;

                        if (!hasMesh)
                        {
                            ImGui::TextDisabled("Load a mesh first to generate a collider.");
                            return;
                        }

                        if (ImGui::Button("Add Collider Box"))
                        {
                            auto& mc = registry.GetComponent<MeshComponent>(e);
                            registry.AddComponent<CollisionComp>(e,
                                CollisionComp{ &ColliderManager::Load(mc.sourcePath, mc.mesh) });
                        }
                        return;
                    }

                    auto& cc = registry.GetComponent<CollisionComp>(e);

                    if (cc.collider)
                    {
                        glm::vec3 bMin = cc.collider->localBox.GetMin();
                        glm::vec3 bMax = cc.collider->localBox.GetMax();
                        glm::vec3 size = cc.collider->localBox.GetSize();

                        ImGui::LabelText("Min", "%.2f  %.2f  %.2f", bMin.x, bMin.y, bMin.z);
                        ImGui::LabelText("Max", "%.2f  %.2f  %.2f", bMax.x, bMax.y, bMax.z);
                        ImGui::LabelText("Size", "%.2f  %.2f  %.2f", size.x, size.y, size.z);

                        if (ImGui::Button("Regenerate"))
                        {
                            if (registry.HasComponent<MeshComponent>(e))
                            {
                                auto& mc = registry.GetComponent<MeshComponent>(e);
                                if (mc.mesh)
                                {
                                    m_vulkan.GetDevice().Get().waitIdle();
                                    ColliderManager::Unload(mc.sourcePath);
                                    cc.collider = &ColliderManager::Load(mc.sourcePath, mc.mesh);
                                }
                            }
                        }

                        ImGui::SameLine();

                        if (ImGui::Button("Remove"))
                            registry.RemoveComponent<CollisionComp>(e);
                    }
                    else
                    {
                        ImGui::TextColored({ 1.0f, 0.3f, 0.3f, 1.0f }, "Collider pointer is null.");
                    }
                });
        }

        void Context::RegisterSerializer()
        {
            m_editorScene.RegisterSerializer<NameComponent>(
                [](SceneEntity e, const SceneRegistry& reg, nlohmann::json& out, const HandleToIndex&)
                {
                    if (!reg.HasComponent<NameComponent>(e))
                        return;

                    out["NameComponent"]["name"] = reg.GetComponent<NameComponent>(e).name;
                },
                [](SceneEntity e, SceneRegistry& reg, const nlohmann::json& in, const IndexToHandle&)
                {
                    if (!in.contains("NameComponent"))
                        return;

                    reg.AddComponent<NameComponent>(e, NameComponent{ in["NameComponent"]["name"] });
                });

            m_editorScene.RegisterSerializer<HierarchyComponent>(
                [](SceneEntity e, const SceneRegistry& reg, nlohmann::json& out, const HandleToIndex& toIndex)
                {
                    if (!reg.HasComponent<HierarchyComponent>(e))
                        return;

                    const auto& h = reg.GetComponent<HierarchyComponent>(e);
                    out["HierarchyComponent"]["parent"] =
                        (h.m_parent == NullEntity) ? -1 : toIndex.at(h.m_parent);
                },
                [](SceneEntity e, SceneRegistry& reg, const nlohmann::json& in, const IndexToHandle& toHandle)
                {
                    if (!in.contains("HierarchyComponent"))
                        return;

                    int idx = in["HierarchyComponent"]["parent"].get<int>();
                    SceneEntity parent = (idx == -1) ? NullEntity : toHandle.at(idx);
                    reg.AddComponent<HierarchyComponent>(e, HierarchyComponent{ parent, {} });
                    if (parent != NullEntity && reg.HasComponent<HierarchyComponent>(parent))
                        reg.GetComponent<HierarchyComponent>(parent).m_children.push_back(e);
                });

            m_editorScene.RegisterSerializer<TransformComponent>(
                [](SceneEntity e, const SceneRegistry& reg, nlohmann::json& out, const HandleToIndex&)
                {
                    if (!reg.HasComponent<TransformComponent>(e))
                        return;

                    const auto& t = reg.GetComponent<TransformComponent>(e);
                    glm::vec3 pos = t.GetPosition();
                    glm::vec3 rot = t.GetRotation();
                    glm::vec3 scale = t.GetScale() * 2.0f;
                    out["TransformComponent"] =
                    {
                        { "pos",   { pos.x,   pos.y,   pos.z   } },
                        { "rot",   { rot.x,   rot.y,   rot.z   } },
                        { "scale", { scale.x, scale.y, scale.z } }
                    };
                },
                [](SceneEntity e, SceneRegistry& reg, const nlohmann::json& in, const IndexToHandle&)
                {
                    if (!in.contains("TransformComponent"))
                        return;

                    const auto& j = in["TransformComponent"];
                    reg.AddComponent<TransformComponent>(e);
                    auto& t = reg.GetComponent<TransformComponent>(e);
                    t.SetPosition({ j["pos"][0],   j["pos"][1],   j["pos"][2] });
                    t.SetRotation({ j["rot"][0],   j["rot"][1],   j["rot"][2] });
                    t.SetScale({    j["scale"][0], j["scale"][1], j["scale"][2] });
                });

            m_editorScene.RegisterSerializer<MeshComponent>(
                [](SceneEntity e, const SceneRegistry& reg, nlohmann::json& out, const HandleToIndex&)
                {
                    if (!reg.HasComponent<MeshComponent>(e))
                        return;

                    const auto& mc = reg.GetComponent<MeshComponent>(e);
                    if (mc.sourcePath.empty())
                        return;

                    out["MeshComponent"]["path"] = mc.sourcePath;
                },
                [this](SceneEntity e, SceneRegistry& reg, const nlohmann::json& in, const IndexToHandle&)
                {
                    if (!in.contains("MeshComponent"))
                        return;

                    std::string path = in["MeshComponent"]["path"].get<std::string>();
                    reg.AddComponent<MeshComponent>(e);
                    auto& mc = reg.GetComponent<MeshComponent>(e);
                    mc.sourcePath = path;
                    mc.mesh = &MeshLoader::Load(path, &m_vulkan);
                });

            m_editorScene.RegisterSerializer<CollisionComp>(
                [](SceneEntity e, const SceneRegistry& reg, nlohmann::json& out, const HandleToIndex&)
                {
                    if (!reg.HasComponent<CollisionComp>(e))
                        return;
                    out["CollisionComp"]["generated"] =
                        (reg.GetComponent<CollisionComp>(e).collider != nullptr);
                },
                [this](SceneEntity e, SceneRegistry& reg, const nlohmann::json& in, const IndexToHandle&)
                {
                    if (!in.contains("CollisionComp"))
                        return;
                    if (!in["CollisionComp"]["generated"].get<bool>())
                        return;
                    if (!reg.HasComponent<MeshComponent>(e))
                        return;

                    auto& mc = reg.GetComponent<MeshComponent>(e);
                    if (!mc.mesh)
                        return;

                    reg.AddComponent<CollisionComp>(e,
                        CollisionComp{ &ColliderManager::Load(mc.sourcePath, mc.mesh) });
                });
        }
    }
}
