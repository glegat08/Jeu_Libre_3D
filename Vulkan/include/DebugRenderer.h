#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Buffer.h"
#include "Pipeline.h"
#include "AABB.h"
#include "OBB.h"
#include "Sphere.h"
#include "VertexDebug.h"
#include "Core/Mesh.h"

namespace KGR
{
	namespace _Vulkan
	{
		class DebugRenderer
		{
		public:
			DebugRenderer() = default;
			DebugRenderer(Device* device, PhysicalDevice* physicalDevice);

            void BeginFrame();
            void DrawAABB(const KGR::AABB3D& box, const glm::vec3& color);
            void DrawOBB(const KGR::OBB3D& box, const glm::vec3& color);
            void DrawSphere(const KGR::Sphere& sphere, const glm::vec3& color);
            void DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);
            void DrawMeshWireframe(const MeshComponent& meshComp, const glm::mat4& model, const glm::vec3& color);

            DebugRenderer& Get();
            const DebugRenderer& Get() const;

            void Upload();
            void Render(vk::raii::CommandBuffer& cmd, KGR::_Vulkan::Pipeline& debugPipeline);

        private:
            Device* m_device;
            PhysicalDevice* m_physicalDevice;

            std::vector<VertexDebug> m_lines;
            Buffer m_debugBuffer = {};
			DebugRenderer* m_debugRenderer = nullptr;
		};
	}
}