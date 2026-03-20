#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Buffer.h"
#include "Pipeline.h"
#include "Math/AABB.h"
#include "Math/OBB.h"
#include "Math/Sphere.h"
#include "VertexDebug.h"
#include "Core/Mesh.h"

namespace KGR
{
	namespace _Vulkan
	{
		/**
		 * @brief Renderer for debug primitives such as lines, boxes, spheres, and wireframe meshes.
		 *
		 * This class allows drawing debug shapes in a Vulkan context and manages
		 * uploading vertex data to a buffer for rendering.
		 */
		class DebugRenderer
		{
		public:

			/**
			 * @brief Default constructor.
			 */
			DebugRenderer() = default;

			/**
			 * @brief Constructs a DebugRenderer with a Vulkan device and physical device.
			 *
			 * @param device Pointer to the logical device.
			 * @param physicalDevice Pointer to the physical device.
			 */
			DebugRenderer(Device* device, PhysicalDevice* physicalDevice);

			/**
			 * @brief Prepares the debug renderer for a new frame.
			 */
			void BeginFrame();

			/**
			 * @brief Adds an axis-aligned bounding box (AABB) to the debug draw list.
			 *
			 * @param box The AABB to draw.
			 * @param color Color of the box.
			 */
			void DrawAABB(const KGR::AABB3D& box, const glm::vec3& color);

			/**
			 * @brief Adds an oriented bounding box (OBB) to the debug draw list.
			 *
			 * @param box The OBB to draw.
			 * @param color Color of the box.
			 */
			void DrawOBB(const KGR::OBB3D& box, const glm::vec3& color);

			/**
			 * @brief Adds a sphere to the debug draw list.
			 *
			 * @param sphere The sphere to draw.
			 * @param color Color of the sphere.
			 */
			void DrawSphere(const KGR::Sphere& sphere, const glm::vec3& color);

			/**
			 * @brief Adds a line to the debug draw list.
			 *
			 * @param a Start point of the line.
			 * @param b End point of the line.
			 * @param color Color of the line.
			 */
			void DrawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);

			/**
			 * @brief Adds a mesh in wireframe mode to the debug draw list.
			 *
			 * @param meshComp Mesh component to draw.
			 * @param model Model transformation matrix.
			 * @param color Color of the mesh wireframe.
			 */
			void DrawMeshWireframe(const MeshComponent& meshComp, const glm::mat4& model, const glm::vec3& color);

			/**
			 * @brief Returns a reference to this DebugRenderer.
			 * @return Reference to this DebugRenderer.
			 */
			DebugRenderer& Get();

			/**
			 * @brief Returns a const reference to this DebugRenderer.
			 * @return Const reference to this DebugRenderer.
			 */
			const DebugRenderer& Get() const;

			/**
			 * @brief Uploads accumulated debug vertices to the GPU buffer.
			 */
			void Upload();

			/**
			 * @brief Renders the uploaded debug primitives using a given command buffer and pipeline.
			 *
			 * @param cmd Command buffer to record draw commands.
			 * @param debugPipeline Pipeline used for rendering debug primitives.
			 */
			void Render(vk::raii::CommandBuffer& cmd, KGR::_Vulkan::Pipeline& debugPipeline);

		private:

			/**
			 * @brief Pointer to the logical device.
			 */
			Device* m_device;

			/**
			 * @brief Pointer to the physical device.
			 */
			PhysicalDevice* m_physicalDevice;

			/**
			 * @brief Container holding debug line vertices.
			 */
			std::vector<VertexDebug> m_lines;

			/**
			 * @brief Vulkan buffer used for debug vertex data.
			 */
			Buffer m_debugBuffer = {};

			/**
			 * @brief Pointer to self for retrieval.
			 */
			DebugRenderer* m_debugRenderer = nullptr;
		};
	}
}