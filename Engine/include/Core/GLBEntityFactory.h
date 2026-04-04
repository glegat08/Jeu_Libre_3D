#pragma once

#include "Core/GLBLoader.h"
#include "Core/AnimationComponent.h"
#include "Core/TrasformComponent.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/Materials.h"

namespace KGR
{
	namespace GLB
	{
		/**
		 * @brief Result of a single entity creation — entity handle and a validity flag.
		 */
		template<typename RegistryT>
		struct GLBEntityResult
		{
			typename RegistryT::type entity{};
			bool valid = false;
		};

		/**
		 * @brief Per-channel texture overrides applied on top of GLB material data.
		 * Any channel left as nullptr keeps whatever the GLB (or neutral fallback) assigned.
		 *
		 * Useful for variants sharing one mesh but differing only in textures (e.g. mob skins):
		 * @code
		 *   Texture& red   = TextureLoader::Load("red.png", app);
		 *   Texture& green = TextureLoader::Load("green.png", app);
		 *
		 *   CreateGLBEntity(registry, *asset, pos1, rot, scale, neutrals, GLBSkinOverride{ .baseColor = &red   });
		 *   CreateGLBEntity(registry, *asset, pos2, rot, scale, neutrals, GLBSkinOverride{ .baseColor = &green });
		 * @endcode
		 */
		struct GLBSkinOverride
		{
			Texture* baseColor = nullptr;
			Texture* pbrMap = nullptr;
			Texture* normalMap = nullptr;
			Texture* emissive = nullptr;
		};

		namespace Detail
		{
			/**
			 * @brief Returns the uploaded texture at @p imageIndex, or @p neutral if absent.
			 */
			inline Texture* ResolveChannel(int imageIndex,
				const std::vector<std::unique_ptr<Texture>>& uploaded, Texture* neutral)
			{
				const bool valid = imageIndex >= 0
					&& imageIndex < static_cast<int>(uploaded.size())
					&& uploaded[imageIndex] != nullptr;

				return valid ? uploaded[imageIndex].get() : neutral;
			}

			/**
			 * @brief Builds a MaterialComponent from the asset's per-primitive material data.
			 * Submesh i corresponds to primitive i; absent channels fall back to @p neutrals.
			 */
			inline MaterialComponent BuildMaterialComponent(const GLBAsset& asset,
				int subMeshCount, const GLBNeutralTextures& neutrals)
			{
				MaterialComponent matComp;
				matComp.SetSize(subMeshCount);

				const auto& primitives = asset.loader.GetPrimitives();
				const auto& materials = asset.loader.GetMaterials();

				for (int i = 0; i < subMeshCount; ++i)
				{
					const int matIdx = (i < static_cast<int>(primitives.size()))
						? primitives[i].materialIndex : -1;

					Material mat;

					if (matIdx >= 0 && matIdx < static_cast<int>(materials.size()))
					{
						const GLBMaterialData& src = materials[matIdx];
						mat.baseColor = ResolveChannel(src.baseColorIndex, asset.textures, neutrals.baseColor);
						mat.pbrMap = ResolveChannel(src.pbrMapIndex, asset.textures, neutrals.pbrMap);
						mat.normalMap = ResolveChannel(src.normalMapIndex, asset.textures, neutrals.normalMap);
						mat.emissive = ResolveChannel(src.emissiveIndex, asset.textures, neutrals.emissive);
					}
					else
					{
						mat.baseColor = neutrals.baseColor;
						mat.pbrMap = neutrals.pbrMap;
						mat.normalMap = neutrals.normalMap;
						mat.emissive = neutrals.emissive;
					}

					matComp.AddMaterial(i, mat);
				}

				return matComp;
			}

			/**
			 * @brief Overwrites channels on every submesh where @p skin specifies a non-null texture.
			 * nullptr channels in the skin are left untouched.
			 */
			inline void ApplySkinOverride(MaterialComponent& matComp, const GLBSkinOverride& skin)
			{
				for (uint32_t i = 0; i < static_cast<uint32_t>(matComp.Size()); ++i)
				{
					Material mat = matComp.GetMaterial(i);

					if (skin.baseColor) mat.baseColor = skin.baseColor;
					if (skin.pbrMap)    mat.pbrMap = skin.pbrMap;
					if (skin.normalMap) mat.normalMap = skin.normalMap;
					if (skin.emissive)  mat.emissive = skin.emissive;

					matComp.AddMaterial(i, mat);
				}
			}

			/**
			 * @brief Registers all ECS components, adding an AnimationComponent when the asset has skeleton data.
			 */
			template<typename RegistryT>
			void RegisterComponents(RegistryT& registry, typename RegistryT::type entity,
				const GLBAsset& asset, MeshComponent meshComp,
				MaterialComponent matComp, TransformComponent transform)
			{
				const auto& skeletons = asset.loader.GetSkeletons();
				const auto& animations = asset.loader.GetAnimations();

				if (!skeletons.empty() && !animations.empty())
				{
					KGR::Animation::AnimationComponent animComp;
					animComp.Init(&skeletons[0], &animations);

					registry.AddComponents(entity,
						std::move(meshComp), std::move(matComp),
						std::move(transform), std::move(animComp));
					return;
				}

				registry.AddComponents(entity,
					std::move(meshComp), std::move(matComp), std::move(transform));
			}
		}

		/**
		 * @brief Instantiates an ECS entity from a pre-loaded GLBAsset.
		 *
		 * Each submesh gets the material the GLB author assigned to it; absent PBR channels
		 * fall back to @p neutrals. Pass a @p skin pointer to override specific texture channels
		 * (useful for colour variants sharing one mesh). nullptr channels in the skin are preserved.
		 *
		 * @param registry  ECS registry to create the entity in.
		 * @param asset     shared asset to instantiate from.
		 * @param position  world-space position.
		 * @param rotation  world-space rotation.
		 * @param scale     world-space scale.
		 * @param neutrals  shared 1x1 GPU textures for absent PBR channels, from GLBCache::GetNeutrals().
		 * @param skin      optional per-channel texture overrides; pass nullptr for the default GLB materials.
		 */
		template<typename RegistryT>
		GLBEntityResult<RegistryT> CreateGLBEntity(RegistryT& registry, const GLBAsset& asset,
			const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale,
			const GLBNeutralTextures& neutrals, const GLBSkinOverride& skin)
		{
			return CreateGLBEntity(registry, asset, position, rotation, scale, neutrals, &skin);
		}

		template<typename RegistryT>
		GLBEntityResult<RegistryT> CreateGLBEntity(RegistryT& registry, const GLBAsset& asset,
			const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale,
			const GLBNeutralTextures& neutrals, const GLBSkinOverride* skin = nullptr)
		{
			GLBEntityResult<RegistryT> result;
			if (!asset.mesh)
				return result;

			MeshComponent meshComp;
			meshComp.mesh = asset.mesh.get();

			TransformComponent transform;
			transform.SetPosition(position);
			transform.SetRotation(rotation);
			transform.SetScale(scale);

			result.entity = registry.CreateEntity();
			result.valid = true;

			auto matComp = Detail::BuildMaterialComponent(asset, 
				static_cast<int>(meshComp.mesh->GetSubMeshesCount()), neutrals);

			if (skin) 
				Detail::ApplySkinOverride(matComp, *skin);

			Detail::RegisterComponents(registry, result.entity, asset,
				std::move(meshComp), std::move(matComp), std::move(transform));

			return result;
		}
	}
}