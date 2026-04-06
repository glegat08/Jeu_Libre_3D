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

class Mesh;
class Texture;
namespace KGR { namespace _Vulkan { class VulkanCore; } }

namespace KGR
{
	namespace GLB
	{
		/** @brief Raw image bytes extracted from a GLB file. */
		struct RawImage
		{
			std::string         m_name;
			std::vector<uint8_t> m_data;
		};

		/**
		 * @brief Image indices for each PBR channel of a glTF material.
		 * -1 means the channel is absent.
		 */
		struct GLBMaterialData
		{
			int baseColorIndex = -1;
			int pbrMapIndex = -1;
			int normalMapIndex = -1;
			int emissiveIndex = -1;
		};

		/** @brief Vertices, indices, and material index for one glTF primitive. */
		struct GLBPrimitive
		{
			std::vector<Vertex>   vertices;
			std::vector<uint32_t> indices;
			int materialIndex = -1;
		};

		/**
		 * @brief Set of 1x1 GPU fallback textures for PBR channels absent from the GLB.
		 * All pointers are owned by GLBCache and valid for its lifetime.
		 */
		struct GLBNeutralTextures
		{
			Texture* baseColor = nullptr;
			Texture* normalMap = nullptr;
			Texture* pbrMap = nullptr;
			Texture* emissive = nullptr;
		};

		/**
		 * @brief Parses geometry, textures, skeletons, and animations from a GLB file.
		 */
		class GLB_Loader
		{
		public:
			GLB_Loader() = default;
			~GLB_Loader() = default;

			static void SetGlobalFilePath(const std::filesystem::path& rootPath);

			/**
			 * @brief Parses a GLB file and populates all internal buffers.
			 * @param filepath path relative to the resource root, or absolute.
			 * @return true on success, false on I/O or parse error.
			 */
			bool Load(const std::string& filepath);

			const std::vector<Vertex>& GetVertices() const;
			const std::vector<uint32_t>& GetIndices() const;
			const std::vector<RawImage>& GetImages() const;
			const std::vector<KGR::Animation::Skeleton>& GetSkeletons()  const;
			const std::vector<KGR::Animation::AnimationClip>& GetAnimations() const;

			/** @brief One entry per glTF primitive, in declaration order. */
			const std::vector<GLBPrimitive>& GetPrimitives() const;

			/** @brief One entry per glTF material, indexed by GLBPrimitive::materialIndex. */
			const std::vector<GLBMaterialData>& GetMaterials()  const;

		private:
			void LoadGeometry(fastgltf::Asset& gltf);
			void LoadTextures(fastgltf::Asset& gltf);
			void LoadMaterials(fastgltf::Asset& gltf);
			void LoadSkeletons(fastgltf::Asset& gltf);
			void LoadAnimations(fastgltf::Asset& gltf);

			std::vector<Vertex> m_vertices;
			std::vector<uint32_t> m_indices;
			std::vector<RawImage> m_images;
			std::vector<GLBPrimitive> m_primitives;
			std::vector<GLBMaterialData> m_materials;
			std::vector<KGR::Animation::Skeleton> m_skeletons;
			std::vector<KGR::Animation::AnimationClip> m_animations;

			std::unordered_map<size_t, int> m_nodeToJointIndex;

			static std::filesystem::path m_absoluteFilePath;
		};

		/**
		 * @brief CPU + GPU data for a single GLB file.
		 * Textures are uploaded once and shared across every entity referencing this asset.
		 */
		struct GLBAsset
		{
			GLB_Loader loader;
			std::unique_ptr<Mesh> mesh;
			std::vector<std::unique_ptr<Texture>> textures;
		};

		/**
		 * @brief Loads each GLB file exactly once and returns the cached asset on subsequent calls.
		 * Call Init() before any Get() to create the shared neutral textures.
		 */
		class GLBCache
		{
		public:
			/**
			 * @brief Creates the four shared 1x1 neutral textures.
			 * @param app Vulkan context used to upload them.
			 */
			void Init(KGR::_Vulkan::VulkanCore* app);

			/**
			 * @brief Returns the asset for @p path, loading it on first access.
			 * @return pointer to the cached asset, or nullptr if loading failed.
			 */
			const GLBAsset* Get(const std::string& path, KGR::_Vulkan::VulkanCore* app);

			/** @brief Raw pointers to the four shared neutral textures — valid for this cache's lifetime. */
			GLBNeutralTextures GetNeutrals() const;

		private:
			std::unordered_map<std::string, std::unique_ptr<GLBAsset>> m_cache;

			std::unique_ptr<Texture> m_neutralBaseColor;
			std::unique_ptr<Texture> m_neutralNormal;
			std::unique_ptr<Texture> m_neutralPbr;
			std::unique_ptr<Texture> m_neutralEmissive;
		};
	}
}