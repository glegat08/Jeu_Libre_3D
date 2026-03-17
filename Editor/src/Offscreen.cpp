#include "Offscreen.h"
#include "Backends/imgui_impl_vulkan.h"
#include <stdexcept>

namespace KGR
{
    namespace Editor
    {
        void Offscreen::Create(VkDevice device, VkPhysicalDevice physDevice, VkDescriptorPool pool,
            uint32_t width, uint32_t height, VkFormat colorFormat, VkFormat depthFormat)
        {
            m_colorFormat = colorFormat;
            CreateResources(device, physDevice, pool, width, height, colorFormat, depthFormat);
        }

        void Offscreen::Resize(VkDevice device, VkPhysicalDevice physDevice, VkDescriptorPool pool,
            uint32_t width, uint32_t height)
        {
            if (m_width == width && m_height == height)
                return;

            Destroy(device);
            CreateResources(device, physDevice, pool, width, height, m_colorFormat, VK_FORMAT_D32_SFLOAT);
        }

        void Offscreen::Destroy(VkDevice device)
        {
            if (m_descriptorSet != VK_NULL_HANDLE)
                ImGui_ImplVulkan_RemoveTexture(m_descriptorSet);
            DestroyResources(device);
        }

        void Offscreen::CreateResources(VkDevice device, VkPhysicalDevice physDevice, VkDescriptorPool pool,
            uint32_t w, uint32_t h, VkFormat colorFmt, VkFormat depthFmt)
        {
            m_width = w;
            m_height = h;

            // color: needs COLOR_ATTACHMENT (Vulkan writes) + SAMPLED (ImGui reads)
            AllocImage(device, physDevice, w, h, colorFmt,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_colorImage, m_colorMemory);
            MakeView(device, m_colorImage, colorFmt, VK_IMAGE_ASPECT_COLOR_BIT, m_colorView);

            // depth: only needs DEPTH_STENCIL_ATTACHMENT, never sampled
            AllocImage(device, physDevice, w, h, depthFmt,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_depthImage, m_depthMemory);
            MakeView(device, m_depthImage, depthFmt, VK_IMAGE_ASPECT_DEPTH_BIT, m_depthView);

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.maxLod = 1.0f;

            if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
                throw std::runtime_error("Offscreen: failed to create sampler");

            // the image must be in SHADER_READ_ONLY_OPTIMAL when ImGui samples it
            m_descriptorSet = ImGui_ImplVulkan_AddTexture(
                m_sampler, m_colorView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        void Offscreen::DestroyResources(VkDevice device)
        {
            vkDestroySampler(device, m_sampler, nullptr);
            vkDestroyImageView(device, m_colorView, nullptr);
            vkFreeMemory(device, m_colorMemory, nullptr);
            vkDestroyImage(device, m_colorImage, nullptr);
            vkDestroyImageView(device, m_depthView, nullptr);
            vkFreeMemory(device, m_depthMemory, nullptr);
            vkDestroyImage(device, m_depthImage, nullptr);

            m_sampler = VK_NULL_HANDLE;
            m_colorView = VK_NULL_HANDLE;
            m_colorMemory = VK_NULL_HANDLE;
            m_colorImage = VK_NULL_HANDLE;
            m_depthView = VK_NULL_HANDLE;
            m_depthMemory = VK_NULL_HANDLE;
            m_depthImage = VK_NULL_HANDLE;
            m_descriptorSet = VK_NULL_HANDLE;
        }

        void Offscreen::AllocImage(VkDevice device, VkPhysicalDevice physDevice,
            uint32_t w, uint32_t h, VkFormat format,
            VkImageUsageFlags usage, VkMemoryPropertyFlags memProps,
            VkImage& outImage, VkDeviceMemory& outMemory)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = format;
            imageInfo.extent = { w, h, 1 };
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = usage;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            if (vkCreateImage(device, &imageInfo, nullptr, &outImage) != VK_SUCCESS)
                throw std::runtime_error("Offscreen: failed to create image");

            VkMemoryRequirements memReqs{};
            vkGetImageMemoryRequirements(device, outImage, &memReqs);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;
            allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReqs.memoryTypeBits, memProps);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &outMemory) != VK_SUCCESS)
                throw std::runtime_error("Offscreen: failed to allocate image memory");

            vkBindImageMemory(device, outImage, outMemory, 0);
        }

        void Offscreen::MakeView(VkDevice device, VkImage image, VkFormat format,
            VkImageAspectFlags aspect, VkImageView& outView)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspect;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr, &outView) != VK_SUCCESS)
                throw std::runtime_error("Offscreen: failed to create image view");
        }

        uint32_t Offscreen::FindMemoryType(VkPhysicalDevice physDevice,
            uint32_t typeFilter, VkMemoryPropertyFlags props)
        {
            VkPhysicalDeviceMemoryProperties memProperties{};
            vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
                if ((typeFilter & (1u << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & props) == props)
                    return i;

            throw std::runtime_error("Offscreen: no suitable memory type found");
        }
    }
}