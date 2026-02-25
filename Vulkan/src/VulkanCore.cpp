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
#include "Core/TrasformComponent.h"
#include "Core/CameraComponent.h"
#include "../../ImGui/include/Backends/imgui_impl_vulkan.h"

KGR::_Vulkan::VulkanCore::VulkanCore(GLFWwindow* window_) : window(window_)
{
}

void KGR::_Vulkan::VulkanCore::initVulkan()
{
	// good Api
	

	// instance creation
	instance = _Vulkan::Instance(AppInfo{},validationLayers);
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
	swapChain = _Vulkan::SwapChain(&physicalDevice, &device, &surface, window, SwapChain::PresMode::Mailbox, 3,nullptr);
	// images view
	swapChainImageViews = _Vulkan::ImagesViews(&swapChain, &device, ImagesViews::ViewType::vt2D);
	// Pipeline
	_Vulkan::ShaderInfo info{
		.ShaderPath = "Shaders/slang.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};
	// Layouts 
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr) };

	auto layout = DescriptorLayout(bindings, &device);
	descriptorSetLayout.Add(std::move(layout));

	graphicsPipeline = _Vulkan::Pipeline(info, &device, &swapChain,&descriptorSetLayout,&physicalDevice,vk::PolygonMode::eFill);
	// Command Buffer
	commandBuffers = _Vulkan::CommandBuffers(&device);

	// load Model
	//LoadModel();
	//// vertex
	//size_t vertSize = vertices.size() * sizeof(vertices[0]);
	//auto vertexTmp = _Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertSize);
	//vertexTmp.MapMemory(vertSize);
	//vertexTmp.Upload(vertices);
	//vertexTmp.UnMapMemory();
	//vertexBuffer = _Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertSize);
	//vertexBuffer.Copy(&vertexTmp, &device, &queue, &commandBuffers);
	//// index
	//size_t indexSize = indices.size() * sizeof(indices[0]);
	//auto indexTmp = _Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexSize);
	//indexTmp.MapMemory(indexSize);
	//indexTmp.Upload(indices);
	//indexTmp.UnMapMemory();
	//indexBuffer = _Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexSize);
	//indexBuffer.Copy(&indexTmp, &device, &queue, &commandBuffers);	
	//

	// SyncObject
	syncObject = SyncObject(&device, swapChain.GetImagesCount());
	// depth Resources
	vk::Format depthFormat = physicalDevice.findSupportedFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto swapChainExtent = swapChain.GetExtend();
	depthImage = Image(swapChainExtent.width, swapChainExtent.height,1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	depthImage.CreateView(depthFormat, vk::ImageAspectFlagBits::eDepth, &device);


	// createImage
	// load Image 
	auto& image = STBManager::Load("Textures\\viking_room.png");
	//auto& image = STBManager::Load("Textures\\BaseTexture.png");

	vk::DeviceSize imageSize = image.width * image.height * 4;

	// record Buffers 
	Buffer buffer = Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, imageSize);
	buffer.MapMemory(imageSize);
	buffer.Upload(image.pixels, imageSize);
	buffer.UnMapMemory();

	uint32_t mipLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;
	textureImage = Image(image.width, image.height, mipLevel, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	transitionImageLayout(textureImage.Get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, textureImage.GetMimMap());
	buffer.CopyImage(&textureImage, &device, &queue, &commandBuffers);
//	transitionImageLayout(textureImage.Get(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, textureImage.GetMimMap());
	textureImage.CreateView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, &device);
	generateMipmaps(textureImage.Get(), vk::Format::eR8G8B8A8Srgb,textureImage.GetWidth(), textureImage.GetHeight(), textureImage.GetMimMap());
	createTextureSampler();


	// UniformBuffer

		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
		auto bufferD = Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferSize);
		bufferD.MapMemory(bufferSize);
		uniformBuffers = std::move(bufferD);
	
	// descriptorPool
	std::vector<vk::DescriptorPoolSize> poolSize{
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, swapChain.GetImagesCount()),
				vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, swapChain.GetImagesCount()) };
	descriptorPool = DescriptorPool(poolSize, 100, &device);
	
	// DescriptorSet
	descriptorSets = DescriptorSet(&device, &descriptorPool, &descriptorSetLayout.Get(0));
	
		vk::DescriptorBufferInfo bufferInfo{
			.buffer = uniformBuffers.Get(),
			.offset = 0,
			.range = sizeof(UniformBufferObject) };
		vk::DescriptorImageInfo imageInfo{
			.sampler = textureSampler,
			.imageView = textureImage.GetView(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };
		std::array descriptorWrites{
			vk::WriteDescriptorSet{
				.dstSet = descriptorSets.Get(),
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &bufferInfo},
			vk::WriteDescriptorSet{
				.dstSet = descriptorSets.Get(),
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo} };
		device.Get().updateDescriptorSets(descriptorWrites, {});


}

void KGR::_Vulkan::VulkanCore::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		_GLFW::Window::PollEvent();
		drawFrame();
	}

	device.Get().waitIdle();
}



void KGR::_Vulkan::VulkanCore::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	
	device.Get().waitIdle();
	swapChain = _Vulkan::SwapChain(&physicalDevice, &device, &surface, window,SwapChain::PresMode::Mailbox ,3, &swapChain);
	swapChainImageViews = _Vulkan::ImagesViews(&swapChain, &device, ImagesViews::ViewType::vt2D);
	_Vulkan::ShaderInfo info{
		.ShaderPath = "Shaders/slang.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};
	graphicsPipeline = _Vulkan::Pipeline(info, &device, &swapChain,&descriptorSetLayout,&physicalDevice, vk::PolygonMode::eFill);

	vk::Format depthFormat = physicalDevice.findSupportedFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto swapChainExtent = swapChain.GetExtend();
	depthImage = Image(swapChainExtent.width, swapChainExtent.height,1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	depthImage.CreateView(depthFormat, vk::ImageAspectFlagBits::eDepth, &device);
}





void KGR::_Vulkan::VulkanCore::recordCommandBuffer(uint32_t imageIndex, vk::raii::CommandBuffer& buffer)
{


	//
	auto& commandBuffer = buffer;
	commandBuffer.begin({});
	// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
	transition_image_layout(
		swapChain.GetImages()[imageIndex],
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
		vk::ImageAspectFlagBits::eColor,commandBuffer);
	// Transition depth image to depth attachment optimal layout
	transition_image_layout(
		*depthImage.Get(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth,commandBuffer);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAttachmentInfo = {
		.imageView = swapChainImageViews.Get()[imageIndex],
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
	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.Get());
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChain.GetExtend().width), static_cast<float>(swapChain.GetExtend().height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChain.GetExtend()));
	commandBuffer.bindVertexBuffers(0, *vertexBuffer.Get(), { 0 });
	commandBuffer.bindIndexBuffer(*indexBuffer.Get(), 0, vk::IndexType::eUint32);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets[syncObject.GetCurrentFrame()].Get(), nullptr);
	commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
	commandBuffer.endRendering();
	// After rendering, transition the swapchain image to PRESENT_SRC
	transition_image_layout(
		swapChain.GetImages()[imageIndex],
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		{},                                                        // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe,                 // dstStage
		vk::ImageAspectFlagBits::eColor,commandBuffer);
	commandBuffer.end();



}

void KGR::_Vulkan::VulkanCore::recordCommandBuffer(uint32_t imageIndex, vk::raii::CommandBuffer& buffer,
	ImDrawData* drawData)
{
	//
	auto& commandBuffer = buffer;
	commandBuffer.begin({});
	// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
	transition_image_layout(
		swapChain.GetImages()[imageIndex],
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
		vk::ImageAspectFlagBits::eColor, commandBuffer);
	// Transition depth image to depth attachment optimal layout
	transition_image_layout(
		*depthImage.Get(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth, commandBuffer);

	vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAttachmentInfo = {
		.imageView = swapChainImageViews.Get()[imageIndex],
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
	commandBuffer.beginRendering(renderingInfo);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.Get());
	commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChain.GetExtend().width), static_cast<float>(swapChain.GetExtend().height), 0.0f, 1.0f));
	commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChain.GetExtend()));
	commandBuffer.bindVertexBuffers(0, *vertexBuffer.Get(), { 0 });
	commandBuffer.bindIndexBuffer(*indexBuffer.Get(), 0, vk::IndexType::eUint32);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets[syncObject.GetCurrentFrame()].Get(), nullptr);
	commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
	
	if (drawData)
		ImGui_ImplVulkan_RenderDrawData(drawData, *commandBuffer);
	
	commandBuffer.endRendering();
	// After rendering, transition the swapchain image to PRESENT_SRC
	transition_image_layout(
		swapChain.GetImages()[imageIndex],
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		{},                                                        // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe,                 // dstStage
		vk::ImageAspectFlagBits::eColor, commandBuffer);
	commandBuffer.end();
}

void KGR::_Vulkan::VulkanCore::LoadModel()
{
	

	auto& obj = TOLManager::Load("Models\\viking_room.obj");


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

void KGR::_Vulkan::VulkanCore::drawFrame()
{
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
	//       while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = device.Get().waitForFences(*syncObject.GetCurrentFence(), vk::True, UINT64_MAX);
	device.Get().resetFences(*syncObject.GetCurrentFence());

	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}

	std::uint32_t result = syncObject.AcquireNextImage(&swapChain,&device);

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain();
		return;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.


	// Only reset the fence if we are submitting work
	auto& buffer = commandBuffers.Acquire(&device);
	buffer.reset();


	updateUniformBuffer(syncObject.GetCurrentFrame());
	recordCommandBuffer(syncObject.GetCurrentImage(), buffer);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const auto submitInfo = vk::SubmitInfo{
			   .waitSemaphoreCount = 1,
			   .pWaitSemaphores = &*syncObject.GetCurrentPresentSemaphore(),
			   .pWaitDstStageMask = &waitDestinationStageMask,
			   .commandBufferCount = 1,
			   .pCommandBuffers = &*buffer,
			   .signalSemaphoreCount = 1,
			   .pSignalSemaphores = &*syncObject.GetCurrentRenderSemaphore(),
	};

	device.Get().resetFences({ commandBuffers.GetFence(buffer) });
	queue.Get().submit(submitInfo, commandBuffers.GetFence(buffer));

	
	queue.Get().submit({}, *syncObject.GetCurrentFence());

	result = PresentImage();
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain();
	}
	commandBuffers.ReleaseCommandBuffer(buffer);
	syncObject.IncrementFrame();
}

void KGR::_Vulkan::VulkanCore::drawFrame(ImDrawData* drawData)
{
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
//       while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = device.Get().waitForFences(*syncObject.GetCurrentFence(), vk::True, UINT64_MAX);
	device.Get().resetFences(*syncObject.GetCurrentFence());

	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}

	std::uint32_t result = syncObject.AcquireNextImage(&swapChain, &device);

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain();
		return;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.


	// Only reset the fence if we are submitting work
	auto& buffer = commandBuffers.Acquire(&device);
	buffer.reset();


	updateUniformBuffer(syncObject.GetCurrentFrame());
	recordCommandBuffer(syncObject.GetCurrentImage(), buffer, drawData);

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const auto submitInfo = vk::SubmitInfo{
			   .waitSemaphoreCount = 1,
			   .pWaitSemaphores = &*syncObject.GetCurrentPresentSemaphore(),
			   .pWaitDstStageMask = &waitDestinationStageMask,
			   .commandBufferCount = 1,
			   .pCommandBuffers = &*buffer,
			   .signalSemaphoreCount = 1,
			   .pSignalSemaphores = &*syncObject.GetCurrentRenderSemaphore(),
	};

	device.Get().resetFences({ commandBuffers.GetFence(buffer) });
	queue.Get().submit(submitInfo, commandBuffers.GetFence(buffer));


	queue.Get().submit({}, *syncObject.GetCurrentFence());

	result = PresentImage();
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain();
	}
	commandBuffers.ReleaseCommandBuffer(buffer);
	syncObject.IncrementFrame();
}


void KGR::_Vulkan::VulkanCore::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout,
                                                     vk::ImageLayout newLayout,uint32_t mipLevels)
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
	queue.Get().submit(submitInfo,nullptr);
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

int KGR::_Vulkan::VulkanCore::BeginRendering(const glm::vec4& color)
{
	// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
	//       while renderFinishedSemaphores is indexed by imageIndex
	auto fenceResult = device.Get().waitForFences(*syncObject.GetCurrentFence(), vk::True, UINT64_MAX);
	device.Get().resetFences(*syncObject.GetCurrentFence());

	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}

	std::uint32_t result = syncObject.AcquireNextImage(&swapChain, &device);

	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain();
		return -1;
	}
	// On other success codes than eSuccess and eSuboptimalKHR we just throw an exception.


	// Only reset the fence if we are submitting work
	m_currentBuffer = &commandBuffers.Acquire(&device);
	m_currentBuffer->reset();
	



	
	m_currentBuffer->begin({});
	// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
	transition_image_layout(
		swapChain.GetImages()[syncObject.GetCurrentImage()],
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},                                                        // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
		vk::ImageAspectFlagBits::eColor, *m_currentBuffer);
	// Transition depth image to depth attachment optimal layout
	transition_image_layout(
		*depthImage.Get(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth, *m_currentBuffer);

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
	m_currentBuffer->beginRendering(renderingInfo);
	m_currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.Get());
	m_currentBuffer->setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChain.GetExtend().width), static_cast<float>(swapChain.GetExtend().height), 0.0f, 1.0f));
	m_currentBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChain.GetExtend()));

	return 0;
}

int KGR::_Vulkan::VulkanCore::EndRendering()
{
	m_currentBuffer->endRendering();
	// After rendering, transition the swapchain image to PRESENT_SRC
	transition_image_layout(
		swapChain.GetImages()[syncObject.GetCurrentImage()],
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		{},                                                        // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe,                 // dstStage
		vk::ImageAspectFlagBits::eColor, *m_currentBuffer);
	m_currentBuffer->end();


	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	const auto submitInfo = vk::SubmitInfo{
			   .waitSemaphoreCount = 1,
			   .pWaitSemaphores = &*syncObject.GetCurrentPresentSemaphore(),
			   .pWaitDstStageMask = &waitDestinationStageMask,
			   .commandBufferCount = 1,
			   .pCommandBuffers = &*(*m_currentBuffer),
			   .signalSemaphoreCount = 1,
			   .pSignalSemaphores = &*syncObject.GetCurrentRenderSemaphore(),
	};

	device.Get().resetFences({ commandBuffers.GetFence(*m_currentBuffer) });
	queue.Get().submit(submitInfo, commandBuffers.GetFence(*m_currentBuffer));


	queue.Get().submit({}, *syncObject.GetCurrentFence());

	auto result = PresentImage();
	// Due to VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS being defined, eErrorOutOfDateKHR can be checked as a result
	// here and does not need to be caught by an exception.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapChain();
		return -1;
	}
	commandBuffers.ReleaseCommandBuffer(*m_currentBuffer);
	syncObject.IncrementFrame();
	device.Get().waitIdle();
	return 0;
}

bool KGR::_Vulkan::VulkanCore::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
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

void KGR::_Vulkan::VulkanCore::RegisterCam(CameraComponent& cam, TransformComponent& transform)
{

	m_ubo = UniformBufferObject{};
	m_ubo->transform = transform.GetFullTransform();
	m_ubo->view = cam.GetView();
	if (cam.GetWidth() != static_cast<float>(swapChain.GetExtend().width) || cam.GetHeight() != static_cast<float>(swapChain.GetExtend().height))
		cam.SetAspect(static_cast<float>(swapChain.GetExtend().width), static_cast<float>(swapChain.GetExtend().height));
	m_ubo->proj = cam.GetProj();
	m_ubo->proj[1][1] *= -1;
}

void KGR::_Vulkan::VulkanCore::RegisterRender(MeshComponent& mesh, TransformComponent& transform)
{
	m_toRenderObject.push_back({transform.GetFullTransform() ,&mesh });
}

void KGR::_Vulkan::VulkanCore::Render(const glm::vec4& color )
{
	if (!m_ubo.has_value())
		throw std::runtime_error("need to register Camera");
	// Update the Camera
	uniformBuffers.Upload(&m_ubo.value(), sizeof(UniformBufferObject));

	int result = 0;
	result = BeginRendering(color);
	if (result == -1)
	{
		m_ubo.reset();
		m_toRenderObject.clear();
		return;
	}
	for (auto& it: m_toRenderObject)
	{
		for (int i = 0; i < it.second->mesh->GetSubMeshesCount(); ++i)
		{
			it.second->mesh->Bind(m_currentBuffer, i);
			m_currentBuffer->pushConstants<glm::mat4>(graphicsPipeline.GetLayout(), vk::ShaderStageFlagBits::eVertex, 0, it.first);
			m_currentBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
			m_currentBuffer->drawIndexed(it.second->mesh->GetSubMesh(i).IndexCount(), 1, 0, 0, 0);
		}
	}
	result = EndRendering();
	if (result == -1)
	{
		m_ubo.reset();
		m_toRenderObject.clear();
		return;
	}
	m_ubo.reset();
	m_toRenderObject.clear();
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


