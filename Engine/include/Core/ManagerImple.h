#pragma once
#include <fstream>
#include <glm/glm.hpp>
#include "Core/RessourcesManager.h"


inline std::unique_ptr<std::ifstream> LoadFile(const std::string& filePath)
{
	auto  file = std::make_unique<std::ifstream>(filePath, std::ios::ate | std::ios::binary);
	if (!file->is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	return std::move(file);
}
using fileManager = KGR::ResourceManager<std::ifstream, KGR::TypeWrapper<>, LoadFile>;

#include "stb_image.h"


struct Image
{
	Image(const std::string& filePath);
	~Image();

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* pixels = nullptr;
	std::string path;

	Image(const Image&) = delete;
    Image(Image&& other) noexcept;
	Image& operator=(const Image&) = delete;
    Image& operator=(Image&& other) noexcept;
};

std::unique_ptr<Image> LoadFile2(const std::string& filePath);
using STBManager = KGR::ResourceManager<Image,KGR::TypeWrapper<>,LoadFile2>;

#define GLM_ENABLE_EXPERIMENTAL
#include <tiny_obj_loader.h>
#include <glm/gtx/hash.hpp>

struct Vertex2
{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		bool operator==(const Vertex2& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}

};

template <>
struct std::hash<Vertex2>
{
	size_t operator()(Vertex2 const& vertex) const noexcept
	{
		return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};
struct Obj
{
	std::vector<Vertex2> vertices;
	std::vector<uint32_t> indices;
};
std::unique_ptr<Obj> LoadFile3(const std::string& filePath);

