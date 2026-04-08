#include "Core/GLBLoader.h"
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "VulkanCore.h"
#include "Math/CollisionComponent.h"

#include <iostream>
#include <algorithm>
#include <variant>
#include <limits>

namespace KGR
{
	namespace GLB
	{
		GLBAsset::GLBAsset() = default;
		GLBAsset::~GLBAsset() = default;

		std::filesystem::path GLB_Loader::m_absoluteFilePath = std::filesystem::path{};

		void GLB_Loader::SetGlobalFilePath(const std::filesystem::path& rootPath)
		{
			m_absoluteFilePath = rootPath;
		}

		bool GLB_Loader::Load(const std::string& filepath)
		{
			std::filesystem::path resolved = filepath;

			if (resolved.is_relative())
			{
				if (m_absoluteFilePath.empty())
				{
					std::cerr << "[GLB_Loader] Resource root not set for relative path: " << filepath << "\n";
					return false;
				}

				resolved = m_absoluteFilePath / resolved;
			}
			resolved = resolved.lexically_normal();

			fastgltf::Parser parser(static_cast<fastgltf::Extensions>(~0ULL));

			auto data = fastgltf::GltfDataBuffer::FromPath(resolved);
			if (data.error() != fastgltf::Error::None)
			{
				std::cerr << "[GLB_Loader] Read error: " << resolved.string()
					<< " | " << fastgltf::getErrorName(data.error())
					<< " | " << fastgltf::getErrorMessage(data.error()) << "\n";

				return false;
			}

			std::filesystem::path dir = resolved.parent_path();
			if (dir.empty())
				dir = std::filesystem::current_path();

			constexpr auto kOptions =
				fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

			auto asset = parser.loadGltfBinary(data.get(), dir, kOptions);
			if (asset.error() != fastgltf::Error::None)
			{
				std::cerr << "[GLB_Loader] Parse error: " << resolved.string()
					<< " | " << fastgltf::getErrorName(asset.error())
					<< " | " << fastgltf::getErrorMessage(asset.error()) << "\n";

				return false;
			}

			fastgltf::Asset& gltf = asset.get();

			m_vertices.clear();
			m_indices.clear();
			m_images.clear();
			m_primitives.clear();
			m_materials.clear();
			m_skeletons.clear();
			m_animations.clear();
			m_nodeToJointIndex.clear();
			m_primitivesPerMesh.clear();
			m_nodeInstances.clear();

			LoadGeometry(gltf);
			LoadTextures(gltf);
			LoadMaterials(gltf);
			LoadSkeletons(gltf);
			LoadAnimations(gltf);
			LoadNodes(gltf);

			return true;
		}

		void GLB_Loader::LoadGeometry(fastgltf::Asset& gltf)
		{
			m_primitivesPerMesh.resize(gltf.meshes.size());

			for (size_t meshIdx = 0; meshIdx < gltf.meshes.size(); ++meshIdx)
			{
				auto& mesh = gltf.meshes[meshIdx];

				for (auto& primitive : mesh.primitives)
				{
					GLBPrimitive prim;
					prim.materialIndex = primitive.materialIndex.has_value()
						? static_cast<int>(*primitive.materialIndex) : -1;

					const size_t globalBase = m_vertices.size();

					auto getAcc = [&](const char* name) -> fastgltf::Accessor*
						{
							auto it = primitive.findAttribute(name);
							return (it != primitive.attributes.end())
								? &gltf.accessors[it->accessorIndex] : nullptr;
						};

					if (auto* acc = getAcc("POSITION"))
					{
						prim.vertices.resize(acc->count);
						m_vertices.resize(globalBase + acc->count);

						fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, *acc,
							[&](glm::vec3 p, size_t i)
							{
								Vertex v{};
								v.pos = p;
								v.normal = { 0, 1, 0 };
								v.weights = { 1, 0, 0, 0 };
								prim.vertices[i] = m_vertices[globalBase + i] = v;
							});
					}

					if (auto* acc = getAcc("NORMAL"))
						fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, *acc,
							[&](glm::vec3 n, size_t i)
							{
								prim.vertices[i].normal = m_vertices[globalBase + i].normal = n;
							});

					if (auto* acc = getAcc("TANGENT"))
						fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, *acc,
							[&](glm::vec4 t, size_t i)
							{
								prim.vertices[i].tangent = m_vertices[globalBase + i].tangent = t;
							});

					if (auto* acc = getAcc("TEXCOORD_0"))
						fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, *acc,
							[&](glm::vec2 uv, size_t i)
							{
								prim.vertices[i].uv = m_vertices[globalBase + i].uv = uv;
							});

					if (auto* acc = getAcc("WEIGHTS_0"))
						fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, *acc,
							[&](glm::vec4 w, size_t i)
							{
								prim.vertices[i].weights = m_vertices[globalBase + i].weights = w;
							});

					if (auto* acc = getAcc("JOINTS_0"))
					{
						auto applyJoints = [&](auto j, size_t i)
							{
								prim.vertices[i].joints =
									m_vertices[globalBase + i].joints = glm::ivec4(j);
							};

						if (acc->componentType == fastgltf::ComponentType::UnsignedByte)
							fastgltf::iterateAccessorWithIndex<glm::u8vec4>(gltf, *acc, applyJoints);
						else
							fastgltf::iterateAccessorWithIndex<glm::u16vec4>(gltf, *acc, applyJoints);
					}

					auto appendLocalIndex = [&](uint32_t idx)
						{
							prim.indices.push_back(idx);
						};
					auto appendGlobalIndex = [&](uint32_t idx)
						{
							m_indices.push_back(idx + static_cast<uint32_t>(globalBase));
						};
					auto appendIndex = [&](uint32_t idx)
						{
							appendLocalIndex(idx);
							appendGlobalIndex(idx);
						};

					if (primitive.indicesAccessor.has_value())
					{
						auto& acc = gltf.accessors[*primitive.indicesAccessor];
						switch (acc.componentType)
						{
						case fastgltf::ComponentType::UnsignedByte:
							fastgltf::iterateAccessor<uint8_t>(gltf, acc,
								[&](uint8_t i) { appendIndex(i); });
							break;
						case fastgltf::ComponentType::UnsignedShort:
							fastgltf::iterateAccessor<uint16_t>(gltf, acc,
								[&](uint16_t i) { appendIndex(i); });
							break;
						default:
							fastgltf::iterateAccessor<uint32_t>(gltf, acc,
								[&](uint32_t i) { appendIndex(i); });
							break;
						}
					}
					else
					{
						for (size_t i = 0; i < prim.vertices.size(); ++i)
							appendIndex(static_cast<uint32_t>(i));
					}

					m_primitivesPerMesh[meshIdx].push_back(prim);
					m_primitives.push_back(std::move(prim));
				}
			}
		}

		void GLB_Loader::LoadNodes(fastgltf::Asset& gltf)
		{
			for (auto& node : gltf.nodes)
			{
				if (!node.meshIndex.has_value())
					continue;

				GLBNodeInstance inst;
				inst.name = node.name.c_str();
				inst.meshIndex = static_cast<int>(*node.meshIndex);

				std::visit(fastgltf::visitor
					{
						[&](const fastgltf::TRS& trs)
						{
							inst.translation =
							{
								trs.translation[0],
								trs.translation[1],
								trs.translation[2]
							};
							// glTF stores quaternions as (x,y,z,w). GLM's constructor is glm::quat(w, x, y, z).
							inst.rotation =
							{
								trs.rotation[3],
								trs.rotation[0],
								trs.rotation[1],
								trs.rotation[2]
							};
							inst.scale =
							{
								trs.scale[0],
								trs.scale[1],
								trs.scale[2]
							};
						},
						[](const auto&) {}
					}, node.transform);

				m_nodeInstances.push_back(std::move(inst));
			}
		}

		void GLB_Loader::LoadTextures(fastgltf::Asset& gltf)
		{
			for (auto& image : gltf.images)
			{
				RawImage rawImg;
				rawImg.m_name = image.name.c_str();

				std::visit(fastgltf::visitor
					{
						[](auto&) {},

						[&](fastgltf::sources::Vector& vector)
						{
							const uint8_t* start =
								reinterpret_cast<const uint8_t*>(vector.bytes.data());
							rawImg.m_data.assign(start, start + vector.bytes.size());
						},

						[&](fastgltf::sources::Array& array)
						{
							const uint8_t* start =
								reinterpret_cast<const uint8_t*>(array.bytes.data());
							rawImg.m_data.assign(start, start + array.bytes.size());
						},

						[&](fastgltf::sources::BufferView& view)
						{
							auto& bufferView = gltf.bufferViews[view.bufferViewIndex];
							auto& buffer = gltf.buffers[bufferView.bufferIndex];

							std::visit(fastgltf::visitor
								{
									[](auto&) {},
									[&](fastgltf::sources::Array& array)
									{
										const uint8_t* start =
											reinterpret_cast<const uint8_t*>(array.bytes.data())
											+ bufferView.byteOffset;
										rawImg.m_data.assign(start, start + bufferView.byteLength);
									}
								}, buffer.data);
						}
					}, image.data);

				m_images.push_back(std::move(rawImg));
			}
		}

		void GLB_Loader::LoadMaterials(fastgltf::Asset& gltf)
		{
			auto resolveTexIndex = [&](const auto& texInfo) -> int
				{
					if (!texInfo.has_value())
						return -1;

					const auto& tex = gltf.textures[texInfo->textureIndex];
					return tex.imageIndex.has_value()
						? static_cast<int>(*tex.imageIndex) : -1;
				};

			for (const auto& mat : gltf.materials)
			{
				GLBMaterialData data;
				data.baseColorIndex = resolveTexIndex(mat.pbrData.baseColorTexture);
				data.pbrMapIndex = resolveTexIndex(mat.pbrData.metallicRoughnessTexture);
				data.normalMapIndex = resolveTexIndex(mat.normalTexture);
				data.emissiveIndex = resolveTexIndex(mat.emissiveTexture);
				m_materials.push_back(data);
			}
		}

		void GLB_Loader::LoadSkeletons(fastgltf::Asset& gltf)
		{
			for (auto& skin : gltf.skins)
			{
				KGR::Animation::Skeleton skeleton;
				skeleton.name = skin.name.c_str();

				for (size_t i = 0; i < skin.joints.size(); ++i)
					m_nodeToJointIndex[skin.joints[i]] = static_cast<int>(i);

				std::vector<glm::mat4> inverseBindMatrices(skin.joints.size(), glm::mat4(1.0f));
				if (skin.inverseBindMatrices.has_value())
				{
					fastgltf::iterateAccessorWithIndex<glm::mat4>(gltf,
						gltf.accessors[*skin.inverseBindMatrices],
						[&](glm::mat4 mat, size_t i) { inverseBindMatrices[i] = mat; });
				}

				for (size_t i = 0; i < skin.joints.size(); ++i)
				{
					auto& node = gltf.nodes[skin.joints[i]];

					KGR::Animation::Joint joint;
					joint.name = node.name.c_str();
					joint.id = static_cast<int>(i);
					joint.inverseBindMatrix = inverseBindMatrices[i];

					std::visit(fastgltf::visitor
						{
							[](const fastgltf::math::fmat4x4&) {},
							[&](const fastgltf::TRS& trs)
							{
								joint.translation =
								{
									trs.translation[0],
									trs.translation[1],
									trs.translation[2]
								};
								joint.rotation =
								{
									trs.rotation[3],
									trs.rotation[0],
									trs.rotation[1],
									trs.rotation[2]
								};
								joint.scale =
								{
									trs.scale[0],
									trs.scale[1],
									trs.scale[2]
								};
							},
							[](const auto&) {}
						}, node.transform);

					for (size_t childNode : node.children)
					{
						auto it = m_nodeToJointIndex.find(childNode);
						if (it != m_nodeToJointIndex.end())
							joint.m_children.push_back(it->second);
					}

					skeleton.m_joints.push_back(joint);
				}

				m_skeletons.push_back(std::move(skeleton));
			}
		}

		void GLB_Loader::LoadAnimations(fastgltf::Asset& gltf)
		{
			for (auto& animation : gltf.animations)
			{
				KGR::Animation::AnimationClip clip;
				clip.name = animation.name.c_str();

				std::unordered_map<int, KGR::Animation::Track> nodeTracks;

				for (auto& channel : animation.channels)
				{
					if (!channel.nodeIndex.has_value())
						continue;

					auto jointIt = m_nodeToJointIndex.find(*channel.nodeIndex);
					if (jointIt == m_nodeToJointIndex.end())
						continue;

					const int jointId = jointIt->second;
					nodeTracks.try_emplace(jointId, KGR::Animation::Track{ jointId });

					auto& sampler = animation.samplers[channel.samplerIndex];
					auto& outputAcc = gltf.accessors[sampler.outputAccessor];
					auto& track = nodeTracks[jointId];

					std::vector<float> times;
					fastgltf::iterateAccessor<float>(gltf, gltf.accessors[sampler.inputAccessor],
						[&](float t)
						{
							times.push_back(t);
							clip.duration = std::max(clip.duration, t);
						});

					if (channel.path == fastgltf::AnimationPath::Translation)
					{
						fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, outputAcc,
							[&](glm::vec3 v, size_t i)
							{
								if (i < times.size())
									track.m_positions.push_back({ times[i], v });
							});
					}
					else if (channel.path == fastgltf::AnimationPath::Rotation)
					{
						fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, outputAcc,
							[&](glm::vec4 v, size_t i)
							{
								if (i < times.size())
									track.m_rotations.push_back({ times[i], { v.w, v.x, v.y, v.z } });
							});
					}
					else if (channel.path == fastgltf::AnimationPath::Scale)
					{
						fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, outputAcc,
							[&](glm::vec3 v, size_t i)
							{
								if (i < times.size())
									track.m_scales.push_back({ times[i], v });
							});
					}
				}

				for (auto& [id, track] : nodeTracks)
					clip.m_tracks.push_back(std::move(track));

				m_animations.push_back(std::move(clip));
			}
		}

		const std::vector<Vertex>& GLB_Loader::GetVertices() const 
		{
			return m_vertices; 
		}

		const std::vector<uint32_t>& GLB_Loader::GetIndices() const 
		{
			return m_indices;
		}

		const std::vector<RawImage>& GLB_Loader::GetImages() const 
		{
			return m_images;
		}

		const std::vector<KGR::Animation::Skeleton>& GLB_Loader::GetSkeletons() const 
		{
			return m_skeletons;
		}

		const std::vector<KGR::Animation::AnimationClip>& GLB_Loader::GetAnimations() const 
		{
			return m_animations; 
		}

		const std::vector<GLBPrimitive>& GLB_Loader::GetPrimitives() const 
		{
			return m_primitives; 
		}

		const std::vector<GLBMaterialData>& GLB_Loader::GetMaterials() const 
		{
			return m_materials;
		}

		const std::vector<std::vector<GLBPrimitive>>& GLB_Loader::GetPrimitivesPerMesh() const 
		{
			return m_primitivesPerMesh; 
		}

		const std::vector<GLBNodeInstance>& GLB_Loader::GetNodeInstances() const 
		{
			return m_nodeInstances; 
		}

		std::unique_ptr<Mesh> LoadMeshFromPrimitives(const std::vector<GLBPrimitive>& primitives,
			KGR::_Vulkan::VulkanCore* vkCore)
		{
			auto mesh = std::make_unique<Mesh>();

			for (size_t i = 0; i < primitives.size(); ++i)
			{
				const auto& prim = primitives[i];
				if (prim.vertices.empty())
					continue;

				auto sub = std::make_unique<SubMeshes>(
					prim.vertices,
					prim.indices,
					"prim_" + std::to_string(i),
					vkCore);

				mesh->AddSubMesh(std::move(sub));
			}

			return mesh;
		}

		static std::unique_ptr<Collider> GenerateColliderFromPrimitives(const std::vector<GLBPrimitive>& primitives)
		{
			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float minZ = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float maxY = std::numeric_limits<float>::lowest();
			float maxZ = std::numeric_limits<float>::lowest();

			for (const auto& prim : primitives)
			{
				for (const auto& v : prim.vertices)
				{
					minX = std::min(minX, v.pos.x);
					minY = std::min(minY, v.pos.y);
					minZ = std::min(minZ, v.pos.z);
					maxX = std::max(maxX, v.pos.x);
					maxY = std::max(maxY, v.pos.y);
					maxZ = std::max(maxZ, v.pos.z);
				}
			}

			auto collider = std::make_unique<Collider>();
			collider->localBox = KGR::AABB3D({ minX, minY, minZ }, { maxX, maxY, maxZ });
			return collider;
		}

		void GLBCache::Init(KGR::_Vulkan::VulkanCore* app)
		{
			static const uint8_t white[4] = { 255, 255, 255, 255 };
			static const uint8_t normal[4] = { 128, 128, 255, 255 };
			static const uint8_t pbr[4] = { 255, 128, 0, 255 };
			static const uint8_t black[4] = { 0, 0, 0, 255 };

			m_neutralBaseColor = LoadTextureRaw(white, 1, 1, app);
			m_neutralNormal = LoadTextureRaw(normal, 1, 1, app);
			m_neutralPbr = LoadTextureRaw(pbr, 1, 1, app);
			m_neutralEmissive = LoadTextureRaw(black, 1, 1, app);
		}

		GLBNeutralTextures GLBCache::GetNeutrals() const
		{
			return
			{
				m_neutralBaseColor.get(),
				m_neutralNormal.get(),
				m_neutralPbr.get(),
				m_neutralEmissive.get()
			};
		}

		const GLBAsset* GLBCache::Get(const std::string& path, KGR::_Vulkan::VulkanCore* app)
		{
			auto it = m_cache.find(path);
			if (it != m_cache.end())
				return it->second.get();

			auto asset = std::make_unique<GLBAsset>();
			if (!asset->loader.Load(path))
				return nullptr;

			asset->mesh = LoadMeshFromGLB(asset->loader, app);

			const auto& perMesh = asset->loader.GetPrimitivesPerMesh();
			asset->meshes.resize(perMesh.size());
			asset->colliders.resize(perMesh.size());

			for (size_t i = 0; i < perMesh.size(); ++i)
			{
				if (perMesh[i].empty())
					continue;

				asset->meshes[i] = LoadMeshFromPrimitives(perMesh[i], app);
				asset->colliders[i] = GenerateColliderFromPrimitives(perMesh[i]);
			}

			asset->textures.reserve(asset->loader.GetImages().size());
			for (const auto& img : asset->loader.GetImages())
			{
				if (img.m_data.empty())
				{
					asset->textures.push_back(nullptr);
					continue;
				}

				try
				{
					asset->textures.push_back(
						LoadTextureFromMemory(img.m_data.data(), img.m_data.size(), app));
				}
				catch (const std::exception& e)
				{
					std::cerr << "[GLBCache] Texture upload failed in '"
						<< path << "': " << e.what() << "\n";
					asset->textures.push_back(nullptr);
				}
			}

			std::cout << "[GLBCache] '" << path << "' -> "
				<< asset->loader.GetPrimitivesPerMesh().size() << " mesh(es), "
				<< asset->loader.GetNodeInstances().size() << " node(s), "
				<< asset->loader.GetImages().size() << " image(s)\n";

			auto [inserted, ok] = m_cache.emplace(path, std::move(asset));
			return inserted->second.get();
		}
	}
}