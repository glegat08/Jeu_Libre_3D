#include "DebugRenderer.h"
#include "Core/Mesh.h"
#include <unordered_set>
#define M_PI 3.14159265358979323846

KGR::_Vulkan::DebugRenderer::DebugRenderer(Device* device, PhysicalDevice* physicalDevice)
    : m_device(device), m_physicalDevice(physicalDevice)
{
}

void KGR::_Vulkan::DebugRenderer::BeginFrame()
{
    m_lines.clear();
}

void KGR::_Vulkan::DebugRenderer::DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
{
    m_lines.push_back({ a, color });
    m_lines.push_back({ b, color });
}

void KGR::_Vulkan::DebugRenderer::DrawAABB(const KGR::AABB3D& box, const glm::vec3& color)
{
    glm::vec3 min = box.GetMin();
    glm::vec3 max = box.GetMax();

    glm::vec3 p[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {max.x, max.y, min.z},
        {min.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z},
        {min.x, max.y, max.z}
    };

    auto add = [&](int a, int b) { DrawLine(p[a], p[b], color); };

    add(0, 1); add(1, 2); add(2, 3); add(3, 0);
    add(4, 5); add(5, 6); add(6, 7); add(7, 4);
    add(0, 4); add(1, 5); add(2, 6); add(3, 7);
}

void KGR::_Vulkan::DebugRenderer::DrawOBB(const KGR::OBB3D& box, const glm::vec3& color)
{
    glm::vec3 c = box.GetCenter();
    glm::vec3 e = box.GetHalfSize();

    glm::vec3 ax = box.GetAxis(0) * e.x;
    glm::vec3 ay = box.GetAxis(1) * e.y;
    glm::vec3 az = box.GetAxis(2) * e.z;

    glm::vec3 p[8] = {
        c - ax - ay - az,
        c + ax - ay - az,
        c + ax + ay - az,
        c - ax + ay - az,
        c - ax - ay + az,
        c + ax - ay + az,
        c + ax + ay + az,
        c - ax + ay + az
    };

    auto add = [&](int a, int b) { DrawLine(p[a], p[b], color); };

    add(0, 1); add(1, 2); add(2, 3); add(3, 0);
    add(4, 5); add(5, 6); add(6, 7); add(7, 4);
    add(0, 4); add(1, 5); add(2, 6); add(3, 7);
}

KGR::_Vulkan::DebugRenderer& KGR::_Vulkan::DebugRenderer::Get()
{
    return *this;
}

const KGR::_Vulkan::DebugRenderer& KGR::_Vulkan::DebugRenderer::Get() const
{
    return *this;
}

void KGR::_Vulkan::DebugRenderer::DrawSphere(const KGR::Sphere& sphere, const glm::vec3& color)
{
    glm::vec3 c = sphere.GetCenter();
    float r = sphere.GetRadius();
    const int N = 32;

    auto circle = [&](glm::vec3 ax1, glm::vec3 ax2)
        {
            for (int i = 0; i < N; i++)
            {
                float a = i * (2 * M_PI / N);
                float b = (i + 1) * (2 * M_PI / N);

                glm::vec3 p1 = c + (ax1 * cos(a) + ax2 * sin(a)) * r;
                glm::vec3 p2 = c + (ax1 * cos(b) + ax2 * sin(b)) * r;

                DrawLine(p1, p2, color);
            }
        };

    circle({ 1,0,0 }, { 0,1,0 });
    circle({ 1,0,0 }, { 0,0,1 });
    circle({ 0,1,0 }, { 0,0,1 });
}

void KGR::_Vulkan::DebugRenderer::DrawMeshWireframe(const MeshComponent& meshComp, const glm::mat4& model, const glm::vec3& color)
{
    Mesh* mesh = meshComp.mesh;
    if (!mesh)
        return;

    struct Edge { uint32_t a, b; };
    std::unordered_set<uint64_t> uniqueEdges;

    auto hashEdge = [](uint32_t a, uint32_t b) {
        uint32_t minV = std::min(a, b);
        uint32_t maxV = std::max(a, b);
        return (uint64_t(minV) << 32) | uint64_t(maxV);
        };

    for (uint32_t s = 0; s < mesh->GetSubMeshesCount(); ++s)
    {
        const SubMeshes& sub = mesh->GetSubMesh(s);
        const auto& vertices = sub.GetVertices();
        const auto& indices = sub.GetIndex();

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            uint64_t e01 = hashEdge(i0, i1);
            uint64_t e12 = hashEdge(i1, i2);
            uint64_t e20 = hashEdge(i2, i0);

            if (uniqueEdges.insert(e01).second)
                DrawLine(model * glm::vec4(vertices[i0].pos, 1.0f),
                    model * glm::vec4(vertices[i1].pos, 1.0f),
                    color);

            if (uniqueEdges.insert(e12).second)
                DrawLine(model * glm::vec4(vertices[i1].pos, 1.0f),
                    model * glm::vec4(vertices[i2].pos, 1.0f),
                    color);

            if (uniqueEdges.insert(e20).second)
                DrawLine(model * glm::vec4(vertices[i2].pos, 1.0f),
                    model * glm::vec4(vertices[i0].pos, 1.0f),
                    color);
        }
    }
}

void KGR::_Vulkan::DebugRenderer::Upload()
{
    if (m_lines.empty())
        return;

    vk::DeviceSize size = m_lines.size() * sizeof(VertexDebug);
    m_debugBuffer = {};
    m_debugBuffer = Buffer(
        m_device,
        m_physicalDevice,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        size
    );

	m_debugBuffer.MapMemory(size);

    m_debugBuffer.Upload(m_lines);
}

void KGR::_Vulkan::DebugRenderer::Render(vk::raii::CommandBuffer& cmd, KGR::_Vulkan::Pipeline& debugPipeline)
{
    if (m_lines.empty())
        return;

    cmd.bindVertexBuffers(0, *m_debugBuffer.Get(), { 0 });
    cmd.draw(m_lines.size(), 1, 0, 0);
}