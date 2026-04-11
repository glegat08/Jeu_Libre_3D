#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>

#include "Vertex.h"
#include "Animation.h"

struct Collider;

class Mesh;
class Texture;
namespace KGR { namespace _Vulkan { class VulkanCore; } }

namespace KGR
{
	namespace GLB
	{
		/** @brief raw image bytes from a GLB file. */
		struct RawImage
		{
			std::string m_name;
			std::vector<uint8_t> m_data;
		};

		/** @brief PBR channel image indices for a glTF material. -1 means absent. */
		struct GLBMaterialData
		{
			int baseColorIndex = -1;
			int pbrMapIndex = -1;
			int normalMapIndex = -1;
			int emissiveIndex = -1;
		};

		/** @brief vertices, indices, and material index for one glTF primitive. */
		struct GLBPrimitive
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			int materialIndex = -1;
		};

		/** @brief 1x1 GPU fallback textures for absent PBR channels. */
		struct GLBNeutralTextures
		{
			Texture* baseColor = nullptr;
			Texture* normalMap = nullptr;
			Texture* pbrMap = nullptr;
			Texture* emissive = nullptr;
		};

		/** @brief glTF node with a mesh reference and its TRS transform. */
		struct GLBNodeInstance
		{
			std::string name;
			int meshIndex = -1;

			glm::vec3 translation{ 0.0f };
			glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
			glm::vec3 scale{ 1.0f };
		};

		/** @brief parses geometry, textures, skeletons, and animations from a GLB file. */
		class GLB_Loader
		{
		public:
			GLB_Loader() = default;
			~GLB_Loader() = default;

			/**
			 * @brief sets the root path used to resolve relative file paths.
			 * @param rootPath absolute path to the resource root.
			 */
			static void SetGlobalFilePath(const std::filesystem::path& rootPath);

			/**
			 * @brief parses a GLB file and populates all internal buffers.
			 * @param filepath path relative to the resource root, or absolute.
			 * @return true on success, false on error.
			 */
			bool Load(const std::string& filepath);

			const std::vector<Vertex>& GetVertices() const;
			const std::vector<uint32_t>& GetIndices() const;
			const std::vector<RawImage>& GetImages() const;
			const std::vector<KGR::Animation::Skeleton>& GetSkeletons() const;
			const std::vector<KGR::Animation::AnimationClip>& GetAnimations() const;
			const std::vector<KGR::Animation::ObjectAnimationClip>& GetObjectAnimations() const;
			const std::vector<GLBMaterialData>& GetMaterials() const;

			/** @brief flat list of all primitives in declaration order. */
			const std::vector<GLBPrimitive>& GetPrimitives() const;

			/** @brief primitives grouped by glTF mesh index. */
			const std::vector<std::vector<GLBPrimitive>>& GetPrimitivesPerMesh() const;

			/** @brief one entry per glTF node that references a mesh. */
			const std::vector<GLBNodeInstance>& GetNodeInstances() const;

		private:
			void LoadGeometry(fastgltf::Asset& gltf);
			void LoadTextures(fastgltf::Asset& gltf);
			void LoadMaterials(fastgltf::Asset& gltf);
			void LoadSkeletons(fastgltf::Asset& gltf);
			void LoadAnimations(fastgltf::Asset& gltf);
			void LoadObjectAnimations(fastgltf::Asset& gltf);
			void LoadNodes(fastgltf::Asset& gltf);

			std::vector<Vertex> m_vertices;
			std::vector<uint32_t> m_indices;
			std::vector<RawImage> m_images;
			std::vector<GLBPrimitive> m_primitives;
			std::vector<GLBMaterialData> m_materials;
			std::vector<KGR::Animation::Skeleton> m_skeletons;
			std::vector<KGR::Animation::AnimationClip> m_animations;
			std::vector<KGR::Animation::ObjectAnimationClip> m_objectAnimations;

			std::vector<std::vector<GLBPrimitive>> m_primitivesPerMesh;
			std::vector<GLBNodeInstance> m_nodeInstances;

			std::unordered_map<size_t, int> m_nodeToJointIndex;

			static std::filesystem::path m_absoluteFilePath;
		};

		/** @brief CPU and GPU data for a single GLB file. */
		struct GLBAsset
		{
			GLB_Loader loader;
			std::unique_ptr<Mesh> mesh;
			std::vector<std::unique_ptr<Mesh>> meshes;
			std::vector<std::unique_ptr<Collider>> colliders;
			std::vector<std::unique_ptr<Texture>> textures;

			GLBAsset();
			~GLBAsset();
		};

		/** @brief loads each GLB file once and returns the cached asset on subsequent calls. */
		class GLBCache
		{
		public:
			/**
			 * @brief creates the shared 1x1 neutral textures.
			 * @param app Vulkan context for texture upload.
			 */
			void Init(KGR::_Vulkan::VulkanCore* app);

			/**
			 * @brief returns the cached asset for @p path, loading it on first access.
			 * @param path file path.
			 * @param app Vulkan context.
			 * @return pointer to the cached asset, or nullptr on failure.
			 */
			const GLBAsset* Get(const std::string& path, KGR::_Vulkan::VulkanCore* app);

			/** @brief returns raw pointers to the four shared neutral textures. */
			GLBNeutralTextures GetNeutrals() const;

		private:
			std::unordered_map<std::string, std::unique_ptr<GLBAsset>> m_cache;

			std::unique_ptr<Texture> m_neutralBaseColor;
			std::unique_ptr<Texture> m_neutralNormal;
			std::unique_ptr<Texture> m_neutralPbr;
			std::unique_ptr<Texture> m_neutralEmissive;
		};

		/**
		 * @brief builds a GPU Mesh from an explicit set of primitives.
		 * @param primitives source primitive data.
		 * @param vkCore Vulkan context for buffer creation.
		 * @return the built GPU mesh.
		 */
		std::unique_ptr<Mesh> LoadMeshFromPrimitives(const std::vector<GLBPrimitive>& primitives,
			KGR::_Vulkan::VulkanCore* vkCore);
	}
}