#include "Viewport.h"
#include "Context.h"
#include "Math/CollisionComponent.h"
#include "Backends/imgui_impl_vulkan.h"
#include <glm/gtc/type_ptr.hpp>
#include <optional>

namespace KGR
{
    namespace Editor
    {
        Viewport::Viewport(Context* context, KGR::_ImGui::ImGuiCore& imgui, KGR::_Vulkan::VulkanCore& vulkanCore)
            : m_context(context), m_ImGui(imgui), m_VulkanCore(vulkanCore)
        {
        }

        void Viewport::Render(Picker& picker, Scene* scene, CameraComponent* cam)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport");

            m_IsFocused = ImGui::IsWindowFocused();
            m_IsHovered = ImGui::IsWindowHovered();

            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            glm::vec2 newSize = { contentSize.x, contentSize.y };

            m_WasResized = (newSize != m_ViewportSizePrev && newSize.x > 1.0f && newSize.y > 1.0f);
            if (m_WasResized)
            {
                m_ViewportSize = newSize;
                m_ViewportSizePrev = newSize;
            }

            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            m_ViewportPos = { cursorPos.x, cursorPos.y };

            if (m_IsFocused)
            {
                if (ImGui::IsKeyPressed(ImGuiKey_W)) m_GizmoMode = GizmoMode::Translate;
                if (ImGui::IsKeyPressed(ImGuiKey_R)) m_GizmoMode = GizmoMode::Rotate;
                if (ImGui::IsKeyPressed(ImGuiKey_C)) m_GizmoMode = GizmoMode::Scale;
            }

            if (m_sceneDescriptor != VK_NULL_HANDLE)
            {
                ImGui::Image(reinterpret_cast<ImTextureID>(m_sceneDescriptor),
                    ImVec2(m_ViewportSize.x, m_ViewportSize.y));
            }
            else
            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(cursorPos,
                    ImVec2(cursorPos.x + contentSize.x, cursorPos.y + contentSize.y),
                    IM_COL32(30, 30, 30, 255));
                dl->AddText(ImVec2(cursorPos.x + 10, cursorPos.y + 10),
                    IM_COL32(180, 180, 180, 255), "Waiting for offscreen target...");
            }

            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                const char* modeLabel =
                    m_GizmoMode == GizmoMode::Translate ? "[Translate]" :
                    m_GizmoMode == GizmoMode::Rotate ? "[Rotate]" : "[Scale]";
                dl->AddText(ImVec2(cursorPos.x + 10, cursorPos.y + 10),
                    IM_COL32(180, 180, 180, 200), "W: Translate | R: Rotate | C: Scale");
                dl->AddText(ImVec2(cursorPos.x + 10, cursorPos.y + 28),
                    IM_COL32(255, 200, 50, 255), modeLabel);
            }

            if (m_IsHovered && cam && scene)
            {
                ImVec2 mp = ImGui::GetMousePos();
                PickInput input;
                input.ray = ScreenToWorld({ mp.x, mp.y }, m_ViewportPos, m_ViewportSize, *cam);
                input.isClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver();
                input.isHeld = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                picker.Update(input, scene, DefaultHitChecker{});
            }

            DrawOutlines(picker, scene, cam);
            DrawGizmo(picker.GetFirst(), scene, cam);

            ImGui::End();
            ImGui::PopStyleVar();
        }

        void Viewport::DrawOutlines(const Picker& picker, Scene* scene, CameraComponent* cam)
        {
            if (!scene || !cam)
                return;

            SceneRegistry& reg = scene->GetRegistry();
            glm::mat4 viewProj = cam->GetProj() * cam->GetView();
            ImDrawList* dl = ImGui::GetWindowDrawList();

            const ImU32 colorSelected = IM_COL32(255, 160, 40, 230);
            const ImU32 colorHovered = IM_COL32(255, 255, 255, 100);

            auto project = [&](glm::vec3 world) -> std::optional<ImVec2>
                {
                    glm::vec4 clip = viewProj * glm::vec4(world, 1.0f);
                    if (clip.w <= 0.001f)
                        return std::nullopt;
                    glm::vec3 ndc = glm::vec3(clip) / clip.w;
                    return ImVec2{
                        (ndc.x * 0.5f + 0.5f) * m_ViewportSize.x + m_ViewportPos.x,
                        (1.0f - (ndc.y * 0.5f + 0.5f)) * m_ViewportSize.y + m_ViewportPos.y
                    };
                };

            // each bit of the corner index picks the sign of one axis
            auto drawOBB = [&](const OBB3D& obb, ImU32 color)
                {
                    glm::vec3 c = obb.GetCenter();
                    glm::vec3 ax = obb.GetAxis(0) * obb.GetHalfSize().x;
                    glm::vec3 ay = obb.GetAxis(1) * obb.GetHalfSize().y;
                    glm::vec3 az = obb.GetAxis(2) * obb.GetHalfSize().z;

                    glm::vec3 corners[8];
                    for (int i = 0; i < 8; ++i)
                        corners[i] = c
                        + ((i & 1) ? ax : -ax)
                        + ((i & 2) ? ay : -ay)
                        + ((i & 4) ? az : -az);

                    static const int edges[12][2] = {
                        {0,1},{1,3},{3,2},{2,0},
                        {4,5},{5,7},{7,6},{6,4},
                        {0,4},{1,5},{2,6},{3,7}
                    };

                    std::optional<ImVec2> projected[8];
                    for (int i = 0; i < 8; ++i)
                        projected[i] = project(corners[i]);

                    for (auto& [a, b] : edges)
                        if (projected[a] && projected[b])
                            dl->AddLine(*projected[a], *projected[b], color, 1.5f);
                };

            auto drawCross = [&](glm::vec3 worldPos, ImU32 color)
                {
                    auto p = project(worldPos);
                    if (!p)
                        return;

                    constexpr float r = 6.0f;
                    dl->AddLine({ p->x - r, p->y }, { p->x + r, p->y }, color, 1.5f);
                    dl->AddLine({ p->x, p->y - r }, { p->x, p->y + r }, color, 1.5f);
                };

            auto tryGetOBB = [&](SceneEntity e) -> std::optional<OBB3D>
                {
                    if (!reg.HasComponent<CollisionComp>(e))
                        return std::nullopt;

                    const auto& cc = reg.GetComponent<CollisionComp>(e);
                    if (!cc.collider)
                        return std::nullopt;

                    auto& t = reg.GetComponent<TransformComponent>(e);
                    return cc.collider->ComputeGlobalOBB(
                        t.GetScale(), t.GetPosition(), t.GetOrientation());
                };

            SceneEntity hovered = picker.GetHovered();
            if (hovered != NullEntity && !picker.isSelected(hovered)
                && reg.HasComponent<TransformComponent>(hovered))
            {
                if (auto obb = tryGetOBB(hovered))
                    drawOBB(*obb, colorHovered);
                else
                    drawCross(reg.GetComponent<TransformComponent>(hovered).GetPosition(), colorHovered);
            }

            for (SceneEntity e : picker.GetSelection())
            {
                if (!reg.HasComponent<TransformComponent>(e))
                    continue;

                if (auto obb = tryGetOBB(e))
                    drawOBB(*obb, colorSelected);
                else
                    drawCross(reg.GetComponent<TransformComponent>(e).GetPosition(), colorSelected);
            }
        }

        void Viewport::DrawGizmo(SceneEntity primary, Scene* scene, CameraComponent* cam)
        {
            if (!scene || !cam || primary == NullEntity)
                return;

            SceneRegistry& reg = scene->GetRegistry();
            if (!reg.HasComponent<TransformComponent>(primary))
                return;

            auto& transform = reg.GetComponent<TransformComponent>(primary);
            const TransformComponent transformBefore = transform;

            glm::mat4 view = cam->GetView();
            glm::mat4 proj = cam->GetProj();
            glm::mat4 model = transform.GetFullTransform();

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(m_ViewportPos.x, m_ViewportPos.y, m_ViewportSize.x, m_ViewportSize.y);

            ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
            switch (m_GizmoMode)
            {
            case GizmoMode::Translate: op = ImGuizmo::TRANSLATE; break;
            case GizmoMode::Rotate:    op = ImGuizmo::ROTATE;    break;
            case GizmoMode::Scale:     op = ImGuizmo::SCALE;     break;
            }

            float mat[16];
            memcpy(mat, glm::value_ptr(model), sizeof(float) * 16);

            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                op, ImGuizmo::LOCAL, mat))
            {
                float t[3], r[3], s[3];
                ImGuizmo::DecomposeMatrixToComponents(mat, t, r, s);

                if (op == ImGuizmo::TRANSLATE)
                    transform.SetPosition({ t[0], t[1], t[2] });
                if (op == ImGuizmo::ROTATE)
                    transform.SetRotation(glm::radians(glm::vec3(r[0], r[1], r[2])));
                if (op == ImGuizmo::SCALE)
                    transform.SetScale(glm::vec3(s[0], s[1], s[2]) * 2.0f);
            }

            const bool gizmoUsing = ImGuizmo::IsUsing();
            if (gizmoUsing && !m_GizmoEditing)
            {
                m_GizmoEditing = true;
                m_GizmoEditedEntity = primary;
                m_GizmoEditedScene = scene;
                m_GizmoInitialTransform = transformBefore;
            }

            if (!gizmoUsing && m_GizmoEditing)
            {
                if (m_context
                    && m_GizmoEditedScene
                    && m_GizmoEditedScene->GetRegistry().HasComponent<TransformComponent>(m_GizmoEditedEntity))
                {
                    auto& finalTransform = m_GizmoEditedScene->GetRegistry().GetComponent<TransformComponent>(m_GizmoEditedEntity);
                    m_context->GetUndoManager().RecordEdit(m_GizmoEditedScene, m_GizmoEditedEntity, m_GizmoInitialTransform, finalTransform);
                }

                m_GizmoEditing = false;
                m_GizmoEditedEntity = NullEntity;
                m_GizmoEditedScene = nullptr;
            }
        }
    }
}