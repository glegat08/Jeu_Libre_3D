#include "Core/Texture.h"
#include "VulkanCore.h"
#include "Core/ManagerImple.h"

Texture::Texture(KGR::_Vulkan::Image&& image, KGR::_Vulkan::DescriptorSet&& set)
	: m_image(std::move(image))
	, m_set(std::move(set))
{
}

void Texture::Bind(const vk::raii::CommandBuffer* buffer, const vk::raii::PipelineLayout* layout, int set)
{
	buffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, set, *m_set.Get(), nullptr);
}

glm::vec2 Texture::GetSize() const
{
	return glm::vec2{ m_image.GetWidth(), m_image.GetHeight() };
}

std::unique_ptr<Texture> LoadTexture(const std::string& filePath, KGR::_Vulkan::VulkanCore* core)
{
	auto image = core->CreateImage(filePath);
	auto set = core->CreateSetForImage(&image);
	return std::make_unique<Texture>(std::move(image), std::move(set));
}

std::unique_ptr<Texture> LoadTextureFromMemory(const uint8_t* data, size_t size, KGR::_Vulkan::VulkanCore* core)
{
	auto image = core->CreateImageFromMemory(data, size);
	auto set = core->CreateSetForImage(&image);
	return std::make_unique<Texture>(std::move(image), std::move(set));
}

std::unique_ptr<Texture> LoadTextureRaw(const uint8_t* rgbaPixels, uint32_t width, uint32_t height, KGR::_Vulkan::VulkanCore* core)
{
	auto image = core->CreateImageRaw(rgbaPixels, width, height);
	auto set = core->CreateSetForImage(&image);
	return std::make_unique<Texture>(std::move(image), std::move(set));
}