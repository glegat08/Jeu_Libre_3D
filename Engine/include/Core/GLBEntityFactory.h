#pragma once

#include "ts_ecs.h"
#include "Core/GLBLoader.h"
#include "Core/AnimationComponent.h"
#include "Core/TrasformComponent.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/Materials.h"
#include "Math/CollisionComponent.h"
#include <iostream>

namespace KGR
{
	namespace GLB
	{
		template<typename RegistryT>
		struct RegistryTraits
		{
			using type = RegistryT::type;
			static type CreateEntity(RegistryT& registry) { return registry.CreateEntity(); }

			template<typename... Components>
			static void AddComponents(RegistryT& registry, type entity, Components&&... components)
			{
				registry.AddComponents(entity, std::forward<Components>(components)...);
			}
		};

		template<>
		struct RegistryTraits<ts::Scene>
		{
			using type = ts::Entity;
			static type CreateEntity(ts::Scene& scene) { return scene.Spawn(); }

			template<typename... Components>
			static void AddComponents(ts::Scene& scene, type entity, Components&&... components)
			{
				(scene.Add<Components>(entity, std::forward<Components>(components)), ...);
			}
		};

		/** @brief entity handle and validity flag returned by entity creation functions. */
		template<typename RegistryT>
		struct GLBEntityResult
		{
			typename RegistryTraits<RegistryT>::type entity{};
			bool valid = false;
		};

		/** @brief per-channel texture overrides applied on top of GLB material data. */
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
			 * @brief returns the uploaded texture at @p imageIndex, or @p neutral if absent.
			 * @param imageIndex image index into the uploaded list.
			 * @param uploaded uploaded GPU textures.
			 * @param neutral fallback texture.
			 * @return resolved texture pointer.
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
			 * @brief builds a MaterialComponent from the asset's flat primitive list.
			 * @param asset source asset.
			 * @param subMeshCount number of submeshes.
			 * @param neutrals fallback textures for absent PBR channels.
			 * @return the built MaterialComponent.
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
			 * @brief builds a MaterialComponent from the primitives of a specific mesh.
			 * @param asset source asset.
			 * @param meshIndex glTF mesh index.
			 * @param subMeshCount number of submeshes.
			 * @param neutrals fallback textures for absent PBR channels.
			 * @return the built MaterialComponent.
			 */
			inline MaterialComponent BuildMaterialComponentForMesh(const GLBAsset& asset,
				int meshIndex, int subMeshCount, const GLBNeutralTextures& neutrals)
			{
				MaterialComponent matComp;
				matComp.SetSize(subMeshCount);

				const auto& perMesh = asset.loader.GetPrimitivesPerMesh();
				const auto& materials = asset.loader.GetMaterials();

				const bool meshValid = meshIndex >= 0
					&& meshIndex < static_cast<int>(perMesh.size());

				for (int i = 0; i < subMeshCount; ++i)
				{
					Material mat;
					int matIdx = -1;

					if (meshValid && i < static_cast<int>(perMesh[meshIndex].size()))
						matIdx = perMesh[meshIndex][i].materialIndex;

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
			 * @brief overwrites material channels where @p skin specifies a non-null texture.
			 * @param matComp material component to modify.
			 * @param skin texture overrides to apply.
			 */
			inline void ApplySkinOverride(MaterialComponent& matComp, const GLBSkinOverride& skin)
			{
				for (uint32_t i = 0; i < static_cast<uint32_t>(matComp.Size()); ++i)
				{
					Material mat = matComp.GetMaterial(i);

					if (skin.baseColor) mat.baseColor = skin.baseColor;
					if (skin.pbrMap) mat.pbrMap = skin.pbrMap;
					if (skin.normalMap) mat.normalMap = skin.normalMap;
					if (skin.emissive) mat.emissive = skin.emissive;

					matComp.AddMaterial(i, mat);
				}
			}

			/**
			 * @brief registers ECS components, adding AnimationComponent if the asset has skeleton data.
			 * @param registry target ECS registry.
			 * @param entity target entity.
			 * @param asset source asset.
			 * @param meshComp mesh component to attach.
			 * @param matComp material component to attach.
			 * @param transform transform component to attach.
			 */
			template<typename RegistryT>
			void RegisterComponents(RegistryT& registry, typename RegistryTraits<RegistryT>::type entity, const GLBAsset& asset,
				MeshComponent meshComp, MaterialComponent matComp, TransformComponent transform)
			{
				const auto& skeletons = asset.loader.GetSkeletons();
				const auto& animations = asset.loader.GetAnimations();

				if (!skeletons.empty() && !animations.empty())
				{
					KGR::Animation::AnimationComponent animComp;
					animComp.Init(&skeletons[0], &animations);

					RegistryTraits<RegistryT>::AddComponents(
						registry, entity,
						std::move(meshComp), std::move(matComp),
						std::move(transform), std::move(animComp));
					return;
				}

				RegistryTraits<RegistryT>::AddComponents(
					registry, entity,
					std::move(meshComp), std::move(matComp), std::move(transform));
			}
		}

		/**
		 * @brief instantiates a single ECS entity from a pre-loaded GLBAsset.
		 * @param registry ECS registry.
		 * @param asset shared asset to instantiate from.
		 * @param position world-space position.
		 * @param rotation world-space rotation in radians.
		 * @param scale world-space scale.
		 * @param neutrals shared 1x1 GPU fallback textures.
		 * @param skin per-channel texture overrides.
		 * @return entity handle and validity flag.
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

			result.entity = RegistryTraits<RegistryT>::CreateEntity(registry);
			result.valid = true;

			auto matComp = Detail::BuildMaterialComponent(
				asset, static_cast<int>(meshComp.mesh->GetSubMeshesCount()), neutrals);

			if (skin)
				Detail::ApplySkinOverride(matComp, *skin);

			Detail::RegisterComponents(registry, result.entity, asset,
				std::move(meshComp), std::move(matComp), std::move(transform));

			return result;
		}

		/**
		 * @brief spawns one ECS entity per glTF node in the asset.
		 * @param registry ECS registry.
		 * @param asset shared asset from GLBCache.
		 * @param worldOffset offset added to every node's translation.
		 * @param neutrals shared 1x1 GPU fallback textures.
		 */
		template<typename RegistryT>
		void CreateGLBEntitiesFromNodes(RegistryT& registry, const GLBAsset& asset,
			const glm::vec3& worldOffset, const GLBNeutralTextures& neutrals)
		{
			const auto& instances = asset.loader.GetNodeInstances();
			const int meshCount = static_cast<int>(asset.meshes.size());

			for (const auto& inst : instances)
			{
				if (inst.meshIndex < 0 || inst.meshIndex >= meshCount)
					continue;

				Mesh* mesh = asset.meshes[inst.meshIndex].get();
				if (!mesh || mesh->GetSubMeshesCount() == 0)
					continue;

				MeshComponent meshComp;
				meshComp.mesh = mesh;

				auto matComp = Detail::BuildMaterialComponentForMesh(
					asset,
					inst.meshIndex,
					static_cast<int>(mesh->GetSubMeshesCount()),
					neutrals);

				TransformComponent transform;
				transform.SetPosition(inst.translation + worldOffset);
				transform.SetOrientation(inst.rotation);
				transform.SetScale(inst.scale);

				Collider* collider = nullptr;
				if (inst.meshIndex < static_cast<int>(asset.colliders.size()))
					collider = asset.colliders[inst.meshIndex].get();

				CollisionComp collComp;
				collComp.collider = collider;

				auto entity = RegistryTraits<RegistryT>::CreateEntity(registry);

				RegistryTraits<RegistryT>::AddComponents(
					registry, entity,
					std::move(meshComp),
					std::move(matComp),
					std::move(transform),
					std::move(collComp));

				std::cout << "[GLB] Node: " << inst.name
					<< " meshIdx=" << inst.meshIndex
					<< " pos=(" << inst.translation.x << ","
					<< inst.translation.y << "," << inst.translation.z << ")\n";
			}
		}
	}
}