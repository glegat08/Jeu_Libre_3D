#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace KGR
{
    namespace Editor
    {
        /** @brief owns the color and depth images that the scene renders into */
        class Offscreen
        {
        public:
            /**
             * @brief allocates all Vulkan resources for the given resolution
             * @param device      logical device
             * @param physDevice  physical device
             * @param pool        descriptor pool
             * @param width       image width in pixels
             * @param height      image height in pixels
             * @param colorFormat swapchain color format
             * @param depthFormat depth buffer format
             */
            void Create(VkDevice device, VkPhysicalDevice physDevice,
                VkDescriptorPool pool, uint32_t width, uint32_t height,
                VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM,
                VkFormat depthFormat = VK_FORMAT_D32_SFLOAT);

            /**
             * @brief destroys and recreates all resources at the new resolution
             * @note does nothing if the size has not changed
             */
            void Resize(VkDevice device, VkPhysicalDevice physDevice,
                VkDescriptorPool pool, uint32_t width, uint32_t height);

            /** @brief frees all Vulkan resources */
            void Destroy(VkDevice device);

            VkImage GetColorImage() const { return m_colorImage; }
            VkImage GetDepthImage() const { return m_depthImage; }
            VkImageView GetColorView() const { return m_colorView; }
            VkImageView GetDepthView() const { return m_depthView; }
            VkSampler GetSampler() const { return m_sampler; }
            VkDescriptorSet GetDescriptorSet() const { return m_descriptorSet; }
            uint32_t GetWidth() const { return m_width; }
            uint32_t GetHeight() const { return m_height; }

            /** @return true if the color image has been created */
            bool IsValid() const { return m_colorImage != VK_NULL_HANDLE; }

        private:
            void CreateResources(VkDevice, VkPhysicalDevice, VkDescriptorPool,
                uint32_t w, uint32_t h, VkFormat colorFmt, VkFormat depthFmt);

            void DestroyResources(VkDevice);

            void AllocImage(VkDevice, VkPhysicalDevice, uint32_t w, uint32_t h,
                VkFormat, VkImageUsageFlags, VkMemoryPropertyFlags,
                VkImage& outImage, VkDeviceMemory& outMemory);

            void MakeView(VkDevice, VkImage, VkFormat, VkImageAspectFlags, VkImageView& outView);

            uint32_t FindMemoryType(VkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags props);

            VkImage m_colorImage = VK_NULL_HANDLE;
            VkDeviceMemory m_colorMemory = VK_NULL_HANDLE;
            VkImageView m_colorView = VK_NULL_HANDLE;
            VkSampler m_sampler = VK_NULL_HANDLE;
            VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
            VkImage m_depthImage = VK_NULL_HANDLE;
            VkDeviceMemory m_depthMemory = VK_NULL_HANDLE;
            VkImageView m_depthView = VK_NULL_HANDLE;
            VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;

            uint32_t m_width = 0;
            uint32_t m_height = 0;
        };
    }
}