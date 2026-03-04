#include "VulkanCore.h"

#include "Core/ManagerImple.h"
#include <algorithm>
#include <array>
#include <assert.h>


#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include "Core/Mesh.h"
#include "Image.h"
#include "_GLFW.h"
#include "../../ImGui/include/imgui.h"
#include "../../ImGui/include/Backends/imgui_impl_vulkan.h"
#include "Core/TrasformComponent.h"
#include "Core/CameraComponent.h"
#include "Core/Texture.h"
#include "../../ImGui/include/Backends/imgui_impl_vulkan.h"


void KGR::_Vulkan::VulkanCore::initVulkan(GLFWwindow* window)
{
	// good Api
	
	// instance creation
	instance = _Vulkan::Instance(AppInfo{}, validationLayers);
	instance.setupDebugMessenger<&debugCallback>();
	//surface 
	surface = _Vulkan::Surface(&instance, window);
	// phDevice
	physicalDevice = _Vulkan::PhysicalDevice(&instance, PhysicalDevice::DeviceType::Best);
	// device
	device = _Vulkan::Device(&physicalDevice, &surface);
	// Queue
	queue = _Vulkan::Queue(&device);
	// swapChain
	swapChain = _Vulkan::SwapChain(&physicalDevice, &device, &surface, window, SwapChain::PresMode::Mailbox, 3, nullptr);
	// images view
	swapChainImageViews = _Vulkan::ImagesViews(&swapChain, &device, ImagesViews::ViewType::vt2D);
	// Pipeline
	_Vulkan::ShaderInfo info{
		.ShaderPath = "Shaders/slang.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};
	_Vulkan::ShaderInfo info2{
		.ShaderPath = "Shaders/slang_line.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};
	// Layouts 
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
	};


	auto layout = DescriptorLayout(bindings, &device);
	descriptorSetLayout.Add(std::move(layout));

	std::vector<vk::DescriptorSetLayoutBinding> bindings2 = {
					vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment, nullptr),
					vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment, nullptr)

	};
	auto layout2 = DescriptorLayout(bindings2, &device);
	descriptorSetLayout.Add(std::move(layout2));


	std::vector<vk::DescriptorSetLayoutBinding> bindings3 = {
					vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
	};
	auto layout3 = DescriptorLayout(bindings3, &device);
	descriptorSetLayout.Add(std::move(layout3));


	graphicsPipeline = _Vulkan::Pipeline(info, &device, &swapChain,&descriptorSetLayout,&physicalDevice,vk::PolygonMode::eFill,Vertex::getBindingDescription(), Vertex::getAttributeDescriptions());
	linePipeLine = _Vulkan::Pipeline(info2, &device, &swapChain, &descriptorSetLayout, &physicalDevice, vk::PolygonMode::eFill, SegmentVertex::getBindingDescription(), SegmentVertex::getAttributeDescriptions());

	// Command Buffer
	commandBuffers = _Vulkan::CommandBuffers(&device);

	
	 
	// SyncObject
	syncObject = SyncObject(&device, swapChain.GetImagesCount());
	// depth Resources
	vk::Format depthFormat = physicalDevice.findSupportedFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto swapChainExtent = swapChain.GetExtend();
	depthImage = Image(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	depthImage.CreateView(depthFormat, vk::ImageAspectFlagBits::eDepth, &device);
	createTextureSampler();


	// UniformBuffer

	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
	auto bufferD = Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferSize);
	bufferD.MapMemory(bufferSize);
	uniformBuffers = std::move(bufferD);


	// light Buffer 
	vk::DeviceSize bufferSize2 = (StorageContainer < LightData, 200 >::Capacity());
	auto bufferD2 = Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferSize2);
	bufferD2.MapMemory(bufferSize2);
	m_lightBuffer = std::move(bufferD2);

	vk::DeviceSize bufferSize3 = sizeof(uint32_t);
	auto bufferD3 = Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferSize3);
	bufferD3.MapMemory(bufferSize3);
	m_lightCount = std::move(bufferD3);

	// descriptorPool
	std::vector<vk::DescriptorPoolSize> poolSize{
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 2),
				vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 497),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer,1)
	};
	descriptorPool = DescriptorPool(poolSize, 500, &device);

	// DescriptorSet
	descriptorSets = DescriptorSet(&device, &descriptorPool, &descriptorSetLayout.Get(0));
	m_LightSet = DescriptorSet(&device, &descriptorPool, &descriptorSetLayout.Get(1));

	vk::DescriptorBufferInfo bufferInfo{
		.buffer = uniformBuffers.Get(),
		.offset = 0,
		.range = uniformBuffers.GetSize() };
	vk::DescriptorBufferInfo bufferInfo2{
	.buffer = m_lightBuffer.Get(),
	.offset = 0,
	.range = m_lightBuffer.GetSize() };
	vk::DescriptorBufferInfo bufferInfo3{
		.buffer = m_lightCount.Get(),
		.offset = 0,
		.range = m_lightCount.GetSize() };
	std::array descriptorWrites{
		vk::WriteDescriptorSet{
			.dstSet = descriptorSets.Get(),
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &bufferInfo},
		vk::WriteDescriptorSet{
				.dstSet = m_LightSet.Get(),
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eStorageBuffer,
				.pBufferInfo = &bufferInfo2},
		vk::WriteDescriptorSet{
				.dstSet = m_LightSet.Get(),
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &bufferInfo},
			vk::WriteDescriptorSet{
					.dstSet = m_LightSet.Get(),
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eStorageBuffer,
					.pBufferInfo = &bufferInfo2},
			vk::WriteDescriptorSet{
					.dstSet = m_LightSet.Get(),
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer,
					.pBufferInfo = &bufferInfo3}
		};
		device.Get().updateDescriptorSets(descriptorWrites, {});



		size_t vertSize =4096 * sizeof(SegmentVertex) * 4;
		stagingVertexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertSize);
		vertexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertSize);
		vertexBuffer.Copy(&stagingVertexBuffer, &device, &queue, &commandBuffers);

		//tmp
		size_t indexSize = 4096 * sizeof(std::uint32_t) * 6;
		stagingIndexBuffer= KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexSize);
		indexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexSize);
		indexBuffer.Copy(&stagingIndexBuffer, &device, &queue, &commandBuffers);
}

// FOR IMGUI
void KGR::_Vulkan::VulkanCore::recreateSwapChain(GLFWwindow* window)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.Get().waitIdle();
	swapChain = _Vulkan::SwapChain(&physicalDevice, &device, &surface, window, SwapChain::PresMode::Mailbox, 3, &swapChain);
	swapChainImageViews = _Vulkan::ImagesViews(&swapChain, &device, ImagesViews::ViewType::vt2D);
	_Vulkan::ShaderInfo info{
		.ShaderPath = "Shaders/slang.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};
	_Vulkan::ShaderInfo info2{
		.ShaderPath = "Shaders/slang_line.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};
	graphicsPipeline = _Vulkan::Pipeline(info, &device, &swapChain,&descriptorSetLayout,&physicalDevice, vk::PolygonMode::eFill,Vertex::getBindingDescription(),Vertex::getAttributeDescriptions());
	linePipeLine = _Vulkan::Pipeline(info2, &device, &swapChain, &descriptorSetLayout, &physicalDevice, vk::PolygonMode::eFill, SegmentVertex::getBindingDescription(), SegmentVertex::getAttributeDescriptions());

	vk::Format depthFormat = physicalDevice.findSupportedFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto swapChainExtent = swapChain.GetExtend();
	depthImage = Image(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	depthImage.CreateView(depthFormat, vk::ImageAspectFlagBits::eDepth, &device);
}

void KGR::_Vulkan::VulkanCore::transition_image_layout(vk::Image image, vk::ImageLayout old_layout,
	vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
	vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask, vk::ImageAspectFlags image_aspect_flags, vk::raii::CommandBuffer& buffer)
{
	vk::ImageMemoryBarrier2 barrier = {
			.srcStageMask = src_stage_mask,
			.srcAccessMask = src_access_mask,
			.dstStageMask = dst_stage_mask,
			.dstAccessMask = dst_access_mask,
			.oldLayout = old_layout,
			.newLayout = new_layout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				   .aspectMask = image_aspect_flags,
				   .baseMipLevel = 0,
				   .levelCount = 1,
				   .baseArrayLayer = 0,
				   .layerCount = 1} };
	vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier };

	buffer.pipelineBarrier2(dependency_info);
}

std::uint32_t KGR::_Vulkan::VulkanCore::PresentImage()
{
	VkSemaphore vkSemaphore = static_cast<VkSemaphore>(*syncObject.GetCurrentRenderSemaphore());

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vkSemaphore;
	presentInfo.swapchainCount = 1;

	VkSwapchainKHR swapchainC = static_cast<VkSwapchainKHR>(*swapChain.Get());
	presentInfo.pSwapchains = &swapchainC;
	presentInfo.pImageIndices = &syncObject.GetCurrentImage();
	presentInfo.pResults = nullptr;

	VkQueue queueC = static_cast<VkQueue>(*queue.Get());
	VkResult result = vkQueuePresentKHR(queueC, &presentInfo);
	return result;
}

void KGR::_Vulkan::VulkanCore::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout, uint32_t mipLevels)
{
	auto& commandBuffer = commandBuffers.Acquire(&device);
	commandBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	vk::ImageMemoryBarrier barrier{ .oldLayout = oldLayout, .newLayout = newLayout, .image = image, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1} };
	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
	commandBuffer.end();
	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
	queue.Get().submit(submitInfo, nullptr);
	queue.Get().waitIdle();
	commandBuffers.ReleaseCommandBuffer(commandBuffer);
}

void KGR::_Vulkan::VulkanCore::generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth,
	int32_t texHeight, uint32_t mipLevels)
{
	// Check if image format supports linear blit-ing
	vk::FormatProperties formatProperties = physicalDevice.Get().getFormatProperties(imageFormat);

	if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	vk::raii::CommandBuffer& commandBuffer = commandBuffers.Acquire(&device);
	vk::CommandBufferBeginInfo beginInfo{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	commandBuffer.begin(beginInfo);

	vk::ImageMemoryBarrier barrier = { .srcAccessMask = vk::AccessFlagBits::eTransferWrite, .dstAccessMask = vk::AccessFlagBits::eTransferRead, .oldLayout = vk::ImageLayout::eTransferDstOptimal, .newLayout = vk::ImageLayout::eTransferSrcOptimal, .srcQueueFamilyIndex = vk::QueueFamilyIgnored, .dstQueueFamilyIndex = vk::QueueFamilyIgnored, .image = image };
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

		vk::ArrayWrapper1D<vk::Offset3D, 2> offsets, dstOffsets;
		offsets[0] = vk::Offset3D(0, 0, 0);
		offsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
		dstOffsets[0] = vk::Offset3D(0, 0, 0);
		dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
		vk::ImageBlit blit = { .srcSubresource = {}, .srcOffsets = offsets, .dstSubresource = {}, .dstOffsets = dstOffsets };
		blit.srcSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
		blit.dstSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1);

		commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
	commandBuffer.end();
	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
	queue.Get().submit(submitInfo, nullptr);
	queue.Get().waitIdle();
	commandBuffers.ReleaseCommandBuffer(commandBuffer);
}



void KGR::_Vulkan::VulkanCore::createTextureSampler()
{
	vk::PhysicalDeviceProperties properties = physicalDevice.Get().getProperties();
	vk::SamplerCreateInfo        samplerInfo{
			   .magFilter = vk::Filter::eLinear,
			   .minFilter = vk::Filter::eLinear,
			   .mipmapMode = vk::SamplerMipmapMode::eLinear,
			   .addressModeU = vk::SamplerAddressMode::eRepeat,
			   .addressModeV = vk::SamplerAddressMode::eRepeat,
			   .addressModeW = vk::SamplerAddressMode::eRepeat,
			   .mipLodBias = 0.0f,
			   .anisotropyEnable = vk::True,
			   .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
			   .compareEnable = vk::False,
			   .compareOp = vk::CompareOp::eAlways,
			   .minLod = 0,
			   .maxLod = vk::LodClampNone };
	textureSampler = vk::raii::Sampler(device.Get(), samplerInfo);
}

int KGR::_Vulkan::VulkanCore::BeginRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer, Pipeline* pipeline,const glm::vec4& color)
{
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
	//       while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = device.Get().waitForFences(*syncObject.GetCurrentFence(), vk::True, UINT64_MAX);
	device.Get().resetFences(*syncObject.GetCurrentFence());

	if (fenceResult != vk::Result::eSuccess)
		throw std::runtime_error("failed to wait for fence!");

	std::uint32_t result = syncObject.AcquireNextImage(&swapChain, &device);

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain(window);
		return -1;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.
	// 
	// Only reset the fence if we are submitting work





	// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
	transition_image_layout(
		swapChain.GetImages()[syncObject.GetCurrentImage()],
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
		vk::ImageAspectFlagBits::eColor, *currentBuffer);
	// Transition depth image to depth attachment optimal layout
	transition_image_layout(
		*depthImage.Get(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth, *currentBuffer);

	vk::ClearValue clearColor = vk::ClearColorValue(color.x, color.y, color.z, color.w);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAttachmentInfo = {
		.imageView = swapChainImageViews.Get()[syncObject.GetCurrentImage()],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor };

	vk::RenderingAttachmentInfo depthAttachmentInfo = {
		.imageView = depthImage.GetView(),
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = clearDepth };

	vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = swapChain.GetExtend()},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo,
		.pDepthAttachment = &depthAttachmentInfo };
	currentBuffer->beginRendering(renderingInfo);
	currentBuffer->setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChain.GetExtend().width), static_cast<float>(swapChain.GetExtend().height), 0.0f, 1.0f));
	currentBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChain.GetExtend()));

	return 0;
}

int KGR::_Vulkan::VulkanCore::EndRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer, const std::vector<vk::Semaphore>& waitS, ImDrawData* imguiDraw)
{
	if (imguiDraw)
		ImGui_ImplVulkan_RenderDrawData(imguiDraw, **currentBuffer);

	currentBuffer->endRendering();

	// After rendering, transition the swapchain image to PRESENT_SRC
	transition_image_layout(
		swapChain.GetImages()[syncObject.GetCurrentImage()],
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		{},                                                        // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe,                 // dstStage
		vk::ImageAspectFlagBits::eColor, *currentBuffer);
	currentBuffer->end();



	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const auto submitInfo = vk::SubmitInfo{
			   .waitSemaphoreCount = static_cast<std::uint32_t>(waitS.size()),
			   .pWaitSemaphores = waitS.data(),
			   .pWaitDstStageMask = &waitDestinationStageMask,
			   .commandBufferCount = 1,
			   .pCommandBuffers = &*(*currentBuffer),
			   .signalSemaphoreCount = 1,
			   .pSignalSemaphores = &*syncObject.GetCurrentRenderSemaphore(),
	};

	device.Get().resetFences({ commandBuffers.GetFence(*currentBuffer) });
	queue.Get().submit(submitInfo, commandBuffers.GetFence(*currentBuffer));


	queue.Get().submit({}, *syncObject.GetCurrentFence());

	auto result = PresentImage();
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain(window);
		return -1;
	}

	return 0;
}


vk::Bool32 KGR::_Vulkan::VulkanCore::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
	if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

KGR::_Vulkan::Image KGR::_Vulkan::VulkanCore::CreateImage(const std::string& filePath)
{
	auto& image = STBManager::Load(filePath);
	vk::DeviceSize imageSize = image.width * image.height * 4;
	KGR::_Vulkan::Buffer buffer = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, imageSize);
	buffer.MapMemory(imageSize);
	buffer.Upload(image.pixels, imageSize);
	buffer.UnMapMemory();
	uint32_t mipLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;
	KGR::_Vulkan::Image textureImage = KGR::_Vulkan::Image(image.width, image.height, mipLevel, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	transitionImageLayout(textureImage.Get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, textureImage.GetMimMap());
	buffer.CopyImage(&textureImage, &device, &queue, &commandBuffers);
	textureImage.CreateView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, &device);
	generateMipmaps(textureImage.Get(), vk::Format::eR8G8B8A8Srgb, textureImage.GetWidth(), textureImage.GetHeight(), textureImage.GetMimMap());

	return textureImage;
}

KGR::_Vulkan::DescriptorSet KGR::_Vulkan::VulkanCore::CreateSetForImage(Image* image)
{
	DescriptorSet set = DescriptorSet(&device, &descriptorPool, &descriptorSetLayout.Get(2));
	vk::DescriptorImageInfo imageInfo{
			.sampler = textureSampler,
			.imageView = image->GetView(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };

	std::array<vk::WriteDescriptorSet, 1> descriptorWrites{
		vk::WriteDescriptorSet{
				.dstSet = set.Get(),
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo}
	};

	device.Get().updateDescriptorSets(descriptorWrites, {});
	return set;
}


void KGR::_Vulkan::VulkanCore::RegisterLight(const LightData& light)
{
	m_lights.push_back(light);
}

void KGR::_Vulkan::VulkanCore::RegisterCam(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
{
	m_ubo = UniformBufferObject{};
	m_ubo->transform = model;
	m_ubo->view = view;
	m_ubo->proj = proj;
	m_ubo->proj[1][1] *= -1;
}

void KGR::_Vulkan::VulkanCore::RegisterRender(Mesh& mesh, const  glm::mat4& model,std::vector<Texture*>& texture)
{
	if (texture.size() != mesh.GetSubMeshesCount())
		throw std::out_of_range("need same amount of subMeshes and texture");
	m_toRenderObject.push_back(MeshData{ model ,&mesh,&texture });
}

void KGR::_Vulkan::VulkanCore::Render(GLFWwindow* window,const glm::vec4& color, ImDrawData* imguiDraw)
{
	if (!m_ubo.has_value())
		throw std::runtime_error("need to register Camera");

	// Update the Camera
	uniformBuffers.Upload(&m_ubo.value(), uniformBuffers.GetSize());
	StorageContainer<LightData, 200> lData = StorageContainer < LightData, 200> ::FromVec(m_lights);
	m_lightBuffer.Upload(lData.Data(), lData.UploadSize());
	m_lightCount.Upload(lData.GetSizeData(), m_lightCount.GetSize());

	auto currentBuffer = &commandBuffers.Acquire(&device);
	currentBuffer->reset();
	currentBuffer->begin({});

	int result = 0;
	result = BeginRendering( window, currentBuffer,&graphicsPipeline,color);
	if (result == -1)
	{
		m_ubo.reset();
		m_toRenderObject.clear();
		m_lights.clear();
		return;
	}
	currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.Get());

	for (auto& it: m_toRenderObject)
	{
		for (int i = 0; i < it.mesh->GetSubMeshesCount(); ++i)
		{
			it.mesh->Bind(currentBuffer, i);
			currentBuffer->pushConstants<glm::mat4>(graphicsPipeline.GetLayout(), vk::ShaderStageFlagBits::eVertex, 0, it.matrixModel);
			currentBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
			currentBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 1, *m_LightSet.Get(), nullptr);
			it.texture->at(i)->Bind(currentBuffer, &graphicsPipeline.GetLayout(), 2);
			currentBuffer->drawIndexed(it.mesh->GetSubMesh(i).IndexCount(), 1, 0, 0, 0);
		}
	}
	currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *linePipeLine.Get());
	currentBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
	currentBuffer->bindVertexBuffers(0, *vertexBuffer.Get(), { 0 });
	currentBuffer->bindIndexBuffer(*indexBuffer.Get(), 0, vk::IndexType::eUint32);

	currentBuffer->drawIndexed(36, 1, 0, 0, 0);


	result = EndRendering(window, currentBuffer,{syncObject.GetCurrentPresentSemaphore()}, imguiDraw);
	commandBuffers.ReleaseCommandBuffer(*currentBuffer);
	syncObject.IncrementFrame();
	device.Get().waitIdle();

	m_ubo.reset();
	m_toRenderObject.clear();
	m_lights.clear(); 
}

KGR::_Vulkan::Instance& KGR::_Vulkan::VulkanCore::GetInstance()
{
	return instance;
}

const KGR::_Vulkan::Instance& KGR::_Vulkan::VulkanCore::GetInstance() const
{
	return instance;
}

KGR::_Vulkan::Surface& KGR::_Vulkan::VulkanCore::GetSurface()
{
	return surface;
}

const KGR::_Vulkan::Surface& KGR::_Vulkan::VulkanCore::GetSurface() const
{
	return surface;
}

KGR::_Vulkan::PhysicalDevice& KGR::_Vulkan::VulkanCore::GetPhysicalDevice()
{
	return physicalDevice;
}

const KGR::_Vulkan::PhysicalDevice& KGR::_Vulkan::VulkanCore::GetPhysicalDevice() const
{
	return physicalDevice;
}

KGR::_Vulkan::Device& KGR::_Vulkan::VulkanCore::GetDevice()
{
	return device;
}

const KGR::_Vulkan::Device& KGR::_Vulkan::VulkanCore::GetDevice() const
{
	return device;
}

KGR::_Vulkan::Queue& KGR::_Vulkan::VulkanCore::GetQueue()
{
	return queue;
}

const KGR::_Vulkan::Queue& KGR::_Vulkan::VulkanCore::GetQueue() const
{
	return queue;
}

KGR::_Vulkan::SwapChain& KGR::_Vulkan::VulkanCore::GetSwapChain()
{
	return swapChain;
}

const KGR::_Vulkan::SwapChain& KGR::_Vulkan::VulkanCore::GetSwapChain() const
{
	return swapChain;
}

KGR::_Vulkan::ImagesViews& KGR::_Vulkan::VulkanCore::GetImagesViews()
{
	return swapChainImageViews;
}

const KGR::_Vulkan::ImagesViews& KGR::_Vulkan::VulkanCore::GetImagesViews() const
{
	return swapChainImageViews;
}

KGR::_Vulkan::Pipeline& KGR::_Vulkan::VulkanCore::GetGraphicsPipeline()
{
	return graphicsPipeline;
}

const KGR::_Vulkan::Pipeline& KGR::_Vulkan::VulkanCore::GetGraphicsPipeline() const
{
	return graphicsPipeline;
}

KGR::_Vulkan::DescriptorPool& KGR::_Vulkan::VulkanCore::GetDescriptorPool()
{
	return descriptorPool;
}

const KGR::_Vulkan::DescriptorPool& KGR::_Vulkan::VulkanCore::GetDescriptorPool() const
{
	return descriptorPool;
}

//IMPL