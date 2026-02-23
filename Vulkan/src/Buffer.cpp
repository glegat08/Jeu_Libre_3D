#include "Buffer.h"
#include "Device.h"
#include "CommandBuffers.h"
#include "Queue.h"
#include "PhysicalDevice.h"
#include "Image.h"

KGR::_Vulkan::Buffer::vkBuffer& KGR::_Vulkan::Buffer::Get()
{
	return m_buffer;
}

const KGR::_Vulkan::Buffer::vkBuffer& KGR::_Vulkan::Buffer::Get() const
{
	return m_buffer;
}

KGR::_Vulkan::Buffer::Buffer(Device* device, PhysicalDevice* phDevice,
	vk::BufferUsageFlags usage, vk::MemoryPropertyFlags MemoryProperties,size_t size)
{
	m_size = size;
	createBuffer(m_size,  usage, MemoryProperties, m_buffer, m_bufferMemory, device, phDevice);
}

void KGR::_Vulkan::Buffer::Upload(const void* data, size_t size)
{	
	if (!dest)
		throw std::runtime_error("Buffer not mapped");
	if (size > m_size)
		throw std::out_of_range("impossible to upload");
	std::memcpy(dest, data, (size_t)size);
}


void KGR::_Vulkan::Buffer::Copy(Buffer* other, Device* device, Queue* queue, CommandBuffers* buffers)
{
	if (other->m_size > m_size)
		throw std::out_of_range("impossible to copy");
	copyBuffer(other->m_buffer, m_buffer, other->m_size , device, queue, buffers);
}

void KGR::_Vulkan::Buffer::CopyImage( Image* image, Device* device, Queue* queue, CommandBuffers* buffers)
{
	vk::raii::CommandBuffer& commandBuffer = buffers->Acquire(device);
	vk::BufferImageCopy  region{ .bufferOffset = 0, .bufferRowLength = 0, .bufferImageHeight = 0, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageOffset = {0, 0, 0}, .imageExtent = {image->GetWidth(), image->GetHeight(), 1} };
	vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	commandBuffer.begin(beginInfo);
	commandBuffer.copyBufferToImage(m_buffer, image->Get(), vk::ImageLayout::eTransferDstOptimal, { region });
	commandBuffer.end();
	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
	queue->Get().submit(submitInfo, nullptr);
	queue->Get().waitIdle();
	buffers->ReleaseCommandBuffer(commandBuffer);
}

void KGR::_Vulkan::Buffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                        vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory,Device*device, PhysicalDevice* phDevice)
{
	vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
	buffer = vk::raii::Buffer(device->Get(), bufferInfo);
	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties,phDevice) };
	bufferMemory = vk::raii::DeviceMemory(device->Get(), allocInfo);
	buffer.bindMemory(bufferMemory, 0);
}


 void KGR::_Vulkan::Buffer::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size, Device* device, Queue* queue, CommandBuffers* buffers)
{
	 vk::raii::CommandBuffer& commandBuffer = buffers->Acquire(device);
	 commandBuffer.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
	commandBuffer.end();
	queue->Get().submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer }, nullptr);
	queue->Get().waitIdle();
	buffers->ReleaseCommandBuffer(commandBuffer);
}

 uint32_t KGR::_Vulkan::Buffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, PhysicalDevice* phDevice)
{
	vk::PhysicalDeviceMemoryProperties memProperties = phDevice->Get().getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void KGR::_Vulkan::Buffer::MapMemory(size_t size)
{
	if (size > m_size)
		throw std::out_of_range("impossible to upload");
	dest = m_bufferMemory.mapMemory(0, size);

}

void KGR::_Vulkan::Buffer::UnMapMemory()
{
	m_bufferMemory.unmapMemory();
	dest = nullptr;
}