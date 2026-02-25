#define STB_IMAGE_IMPLEMENTATION


#include "Core/ManagerImple.h"


Image::Image(const std::string& filePath) : path(filePath)
{
	pixels = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
}

Image::~Image()
{
	stbi_image_free(pixels);
}

Image::Image(Image&& other) noexcept: pixels(other.pixels),
                                      width(other.width),
                                      height(other.height),
                                      channels(other.channels)
{
	other.pixels = nullptr;
}

Image& Image::operator=(Image&& other) noexcept
{
	if (this == &other) 
		return *this;

	if (pixels)
		stbi_image_free(pixels);

	pixels = other.pixels;
	width = other.width;
	height = other.height;
	channels = other.channels;

	other.pixels = nullptr;
	return *this;
}

std::unique_ptr<Image> LoadFile2(const std::string& filePath)
{

	return std::make_unique<Image>(filePath);
}



std::unique_ptr<Obj> LoadFile3(const std::string& filePath)
{
	std::unique_ptr<Obj> result = std::make_unique<Obj>();
	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      warn, err;

	if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex2, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex2 vertex{};

			// store like this X0,Y0,Z0 so 3 * index.vertex_index + the offset
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2] };

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

			vertex.color = { 1.0f, 1.0f, 1.0f };

			// if vertex is unique add it 
			if (!uniqueVertices.contains(vertex))
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(result->vertices.size());
				result->vertices.push_back(vertex);
			}
			// add the index
			result->indices.push_back(uniqueVertices[vertex]);
		}
	}
	return std::move(result);
}
