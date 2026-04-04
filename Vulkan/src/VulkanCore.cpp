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
	_Vulkan::ShaderInfo info3{
		.ShaderPath = "Shaders/slang_ui.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};

	// Layouts 
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
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
	std::vector<vk::DescriptorSetLayoutBinding> bindings4 = {
				vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
	};
	std::vector<vk::DescriptorSetLayoutBinding> bindings5 = {
				vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
	};
	std::vector<vk::DescriptorSetLayoutBinding> bindings6 = {
				vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
	};
	auto layout3 = DescriptorLayout(bindings3, &device);
	auto layout4 = DescriptorLayout(bindings3, &device);
	auto layout5 = DescriptorLayout(bindings4, &device);
	auto layout6 = DescriptorLayout(bindings5, &device);
	auto layout7 = DescriptorLayout(bindings6, &device);
	descriptorSetLayout.Add(std::move(layout3));
	descriptorSetLayout.Add(std::move(layout5));
	descriptorSetLayout.Add(std::move(layout6));
	descriptorSetLayout.Add(std::move(layout7));

	uiLayout.Add(std::move(layout4));

	graphicsPipeline = _Vulkan::Pipeline(info, &device, &swapChain,&descriptorSetLayout,&physicalDevice,vk::PolygonMode::eFill,Vertex::getBindingDescription(), Vertex::getAttributeDescriptions());
	linePipeLine = _Vulkan::Pipeline(info2, &device, &swapChain, &descriptorSetLayout, &physicalDevice, vk::PolygonMode::eFill, SegmentVertex::getBindingDescription(), SegmentVertex::getAttributeDescriptions());
	uiPipeline = _Vulkan::Pipeline::CreateUiPipeline(info3, &device, &swapChain, &uiLayout, &physicalDevice, Vertex2D::getBindingDescription(), Vertex2D::getAttributeDescriptions());
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


	// BOne buffer
	m_frameBones.reserve(10000);
	vk::DeviceSize boneBufferSize = sizeof(glm::mat4) * 10000;
	auto boneBufferTmp = Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, boneBufferSize);
	boneBufferTmp.MapMemory(boneBufferSize);
	m_boneBuffer = std::move(boneBufferTmp);

	std::vector<glm::mat4> identityBones(10000, glm::mat4(1.0f));
	m_boneBuffer.Upload(identityBones.data(), identityBones.size() * sizeof(glm::mat4));

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
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 3),
				vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 497),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 2)
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
	vk::DescriptorBufferInfo boneBufferInfo{
	.buffer = m_boneBuffer.Get(),
	.offset = 0,
	.range = m_boneBuffer.GetSize()};
	std::array descriptorWrites
	{
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
				.descriptorType = vk::DescriptorType::eStorageBuffer,
				.pBufferInfo = &boneBufferInfo},
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
					.pBufferInfo = &bufferInfo3
				}
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

		std::vector<Vertex2D> vertices =
		{
			{{-0.5f, -0.5f}, {0.0f, 0.0f}}, 
			{{ 0.5f, -0.5f}, {1.0f, 0.0f}}, 
			{{ 0.5f,  0.5f}, {1.0f, 1.0f}}, 
			{{-0.5f,  0.5f}, {0.0f, 1.0f}}, 
		};
	std::vector<std::uint32_t> indices =
	{
		0, 2, 1,
		2, 0, 3
	};

		uiVertexBuffer = CreateVertexBuffer(vertices);
		uiIndexBuffer = CreateIndexBuffer(indices);
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
	_Vulkan::ShaderInfo info3{
		.ShaderPath = "Shaders/slang_ui.spv",
		.vertexMain = "vertMain",
		.fragmentMain = "fragMain"
	};

	graphicsPipeline = _Vulkan::Pipeline(info, &device, &swapChain,&descriptorSetLayout,&physicalDevice, vk::PolygonMode::eFill,Vertex::getBindingDescription(),Vertex::getAttributeDescriptions());
	linePipeLine = _Vulkan::Pipeline(info2, &device, &swapChain, &descriptorSetLayout, &physicalDevice, vk::PolygonMode::eFill, SegmentVertex::getBindingDescription(), SegmentVertex::getAttributeDescriptions());
	uiPipeline = _Vulkan::Pipeline::CreateUiPipeline(info3, &device, &swapChain, &uiLayout, &physicalDevice, Vertex2D::getBindingDescription(), Vertex2D::getAttributeDescriptions());

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

void KGR::_Vulkan::VulkanCore::RenderSceneToOffscreen(KGR::Editor::Offscreen& target, const glm::vec4& clearColor, vk::raii::CommandBuffer* cmd)
{
	// Transition 1: color image UNDEFINED → COLOR_ATTACHMENT_OPTIMAL
	// (UNDEFINED is fine here; we clear on load so previous contents don't matter.)
	transition_image_layout(
		target.GetColorImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eTopOfPipe,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::ImageAspectFlagBits::eColor, *cmd);

	// Transition 2: depth image UNDEFINED → DEPTH_ATTACHMENT_OPTIMAL
	transition_image_layout(
		target.GetDepthImage(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eTopOfPipe,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth, *cmd);

	vk::ClearValue clearCol = vk::ClearColorValue(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	vk::ClearValue clearDep = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAtt = {
		.imageView = static_cast<vk::ImageView>(target.GetColorView()),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearCol };

	vk::RenderingAttachmentInfo depthAtt = {
		.imageView = static_cast<vk::ImageView>(target.GetDepthView()),
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = clearDep };

	vk::RenderingInfo ri = {
		.renderArea = {.offset = {0, 0},
									.extent = { target.GetWidth(), target.GetHeight() } },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAtt,
		.pDepthAttachment = &depthAtt };

	cmd->beginRendering(ri);
	cmd->setViewport(0, vk::Viewport(
		0.0f, 0.0f,
		static_cast<float>(target.GetWidth()),
		static_cast<float>(target.GetHeight()),
		0.0f, 1.0f));
	cmd->setScissor(0, vk::Rect2D({ 0, 0 }, { target.GetWidth(), target.GetHeight() }));

	// Draw scene objects (same loop as the swapchain path).
	cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.Get());
	for (auto& it : m_toRenderObject)
	{
		for (int i = 0; i < it.mesh->GetSubMeshesCount(); ++i)
		{
			it.mesh->Bind(cmd, i);
			cmd->pushConstants<glm::mat4>(
				graphicsPipeline.GetLayout(), vk::ShaderStageFlagBits::eVertex, 0, it.matrixModel);
			cmd->bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
			cmd->bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 1, *m_LightSet.Get(), nullptr);
			it.texture.at(i).baseColor->Bind(cmd, &graphicsPipeline.GetLayout(), 2);
			it.texture.at(i).pbrMap->Bind(cmd, &graphicsPipeline.GetLayout(), 3);
			it.texture.at(i).emissive->Bind(cmd, &graphicsPipeline.GetLayout(), 4);
			it.texture.at(i).normalMap->Bind(cmd, &graphicsPipeline.GetLayout(), 5);
			cmd->drawIndexed(it.mesh->GetSubMesh(i).IndexCount(), 1, 0, 0, 0);
		}
	}

	// Debug lines also go into the viewport.
	cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, *linePipeLine.Get());
	cmd->bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
	cmd->bindVertexBuffers(0, *vertexBuffer.Get(), { 0 });
	cmd->bindIndexBuffer(*indexBuffer.Get(), 0, vk::IndexType::eUint32);
	cmd->drawIndexed(36, 1, 0, 0, 0);

	cmd->endRendering();

	// Transition 3: color image COLOR_ATTACHMENT_OPTIMAL → SHADER_READ_ONLY_OPTIMAL.
	// This barrier is what makes ImGui::Image() safe: it guarantees that by the time
	// the fragment shader in the ImGui draw call samples this image, all writes are done.
	transition_image_layout(
		target.GetColorImage(),
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::ImageAspectFlagBits::eColor, *cmd);
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
	KGR::_Vulkan::Image textureImage = KGR::_Vulkan::Image(image.width, image.height, mipLevel, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, &device, &physicalDevice);
	transitionImageLayout(textureImage.Get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, textureImage.GetMimMap());
	buffer.CopyImage(&textureImage, &device, &queue, &commandBuffers);
	textureImage.CreateView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, &device);
	generateMipmaps(textureImage.Get(), vk::Format::eR8G8B8A8Unorm, textureImage.GetWidth(), textureImage.GetHeight(), textureImage.GetMimMap());

	return textureImage;
}

KGR::_Vulkan::Image KGR::_Vulkan::VulkanCore::CreateImageFromMemory(const unsigned char* data, size_t size)
{
	int texWidth, texHeight, texChannels;

	std::unique_ptr<stbi_uc, StbiCleaner> pixels(
		stbi_load_from_memory(data, static_cast<int>(size), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)
	);

	if (!pixels) 
	{
		throw std::runtime_error(std::string("[VulkanCore] CreateImageFromMemory : stbi decode failed - ") + stbi_failure_reason());
	}

	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	KGR::_Vulkan::Buffer stagingBuffer(
		&device, &physicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		imageSize
	);

	stagingBuffer.MapMemory(imageSize);
	stagingBuffer.Upload(pixels.get(), imageSize);
	stagingBuffer.UnMapMemory();

	uint32_t mipLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	KGR::_Vulkan::Image textureImage(
		texWidth, texHeight, mipLevel,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		&device, &physicalDevice
	);

	transitionImageLayout(textureImage.Get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, textureImage.GetMimMap());
	stagingBuffer.CopyImage(&textureImage, &device, &queue, &commandBuffers);
	textureImage.CreateView(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, &device);
	generateMipmaps(textureImage.Get(), vk::Format::eR8G8B8A8Unorm, textureImage.GetWidth(), textureImage.GetHeight(), textureImage.GetMimMap());

	return textureImage;
}

KGR::_Vulkan::Image KGR::_Vulkan::VulkanCore::CreateImageRaw(const uint8_t* rgbaPixels, uint32_t width, uint32_t height)
{
	vk::DeviceSize imageSize = width * height * 4;

	KGR::_Vulkan::Buffer stagingBuffer(
		&device, &physicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		imageSize);

	stagingBuffer.MapMemory(imageSize);
	stagingBuffer.Upload(rgbaPixels, imageSize);
	stagingBuffer.UnMapMemory();

	uint32_t mipLevel = static_cast<uint32_t>(
		std::floor(std::log2(std::max(width, height)))) + 1;

	KGR::_Vulkan::Image textureImage(
		width, height, mipLevel,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferSrc |
		vk::ImageUsageFlagBits::eTransferDst |
		vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		&device, &physicalDevice);

	transitionImageLayout(textureImage.Get(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal, mipLevel);

	stagingBuffer.CopyImage(&textureImage, &device, &queue, &commandBuffers);

	textureImage.CreateView(vk::Format::eR8G8B8A8Unorm,
		vk::ImageAspectFlagBits::eColor, &device);

	generateMipmaps(textureImage.Get(), vk::Format::eR8G8B8A8Unorm,
		textureImage.GetWidth(), textureImage.GetHeight(), mipLevel);

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

int KGR::_Vulkan::VulkanCore::RegisterBoneMatrices(const std::vector<glm::mat4>& boneMatrices)
{
	if (boneMatrices.empty())
		return -1;

	if (m_frameBones.size() + boneMatrices.size() > 10000)
	{
		std::cerr << "[VulkanCore] WARNING : Maximum bones capacity is reached for this frame !" << std::endl;
		return -1;
	}

	int currentOffset = static_cast<int>(m_frameBones.size());
	m_frameBones.insert(m_frameBones.end(), boneMatrices.begin(), boneMatrices.end());

	return currentOffset;
}

void KGR::_Vulkan::VulkanCore::RegisterRender(Mesh& mesh, const glm::mat4& model, const std::vector<Material>& texture, int boneOffset)
{
	if (texture.size() != mesh.GetSubMeshesCount())
		throw std::out_of_range("need same amount of subMeshes and texture");

	m_toRenderObject.push_back(MeshData{ model, &mesh, texture, boneOffset });
}


void KGR::_Vulkan::VulkanCore::RegisterUi(const UiData& data, Texture* texture,const glm::vec2& screenSize)
{
	auto valid = data.GetValid();
	valid.raw1[3] = screenSize.x;
	valid.raw2[3] = screenSize.y;
	uIRender.emplace_back( texture, valid);
}

void KGR::_Vulkan::VulkanCore::Render(GLFWwindow* window, const glm::vec4& color, ImDrawData* imguiDraw, KGR::Editor::Offscreen* offscreen)
{
	if (!m_ubo.has_value())
		throw std::runtime_error("need to register Camera");

	uniformBuffers.Upload(&m_ubo.value(), uniformBuffers.GetSize());
	StorageContainer<LightData, 200> lData = StorageContainer<LightData, 200>::FromVec(m_lights);
	m_lightBuffer.Upload(lData.Data(), lData.UploadSize());
	m_lightCount.Upload(lData.GetSizeData(), m_lightCount.GetSize());

	if (!m_frameBones.empty())
	{
		size_t dataSize = m_frameBones.size() * sizeof(glm::mat4);
		m_boneBuffer.Upload(m_frameBones.data(), dataSize);
	}

	auto currentBuffer = &commandBuffers.Acquire(&device);
	currentBuffer->reset();
	currentBuffer->begin({});

	// --- OFFSCREEN PASS ---------------------------------------------------
	// When an OffscreenTarget is provided we render the 3D scene into it
	// BEFORE acquiring the swapchain image.  Both passes share the same
	// command buffer so a single submit covers everything.
	if (offscreen && offscreen->IsValid())
	{
		RenderSceneToOffscreen(*offscreen, color, currentBuffer);
	}

	// --- SWAPCHAIN PASS ---------------------------------------------------
	// If we rendered to offscreen, the swapchain only needs the ImGui layer.
	// Pass a neutral clear colour so we don't see the scene painted twice.
	glm::vec4 swapchainClear = offscreen ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : color;

	int result = BeginRendering(window, currentBuffer, &graphicsPipeline, swapchainClear);
	if (result == -1)
	{
		m_ubo.reset();
		m_toRenderObject.clear();
		m_lights.clear();
		uIRender.clear();
		return;
	}

	// Legacy path: no offscreen target → draw the scene to the swapchain directly.
	if (!offscreen)
	{
		currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.Get());

		for (auto& it : m_toRenderObject)
		{
			for (int i = 0; i < it.mesh->GetSubMeshesCount(); ++i)
			{
				it.mesh->Bind(currentBuffer, i);

				PushConstantData pcData;
				pcData.model = it.matrixModel;
				pcData.boneOffset = it.boneOffset;

				currentBuffer->pushConstants<PushConstantData>(
					graphicsPipeline.GetLayout(), vk::ShaderStageFlagBits::eVertex, 0, pcData);
				currentBuffer->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
				currentBuffer->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 1, *m_LightSet.Get(), nullptr);
				it.texture.at(i).baseColor->Bind(currentBuffer, &graphicsPipeline.GetLayout(), 2);
				it.texture.at(i).pbrMap->Bind(currentBuffer, &graphicsPipeline.GetLayout(), 3);
				it.texture.at(i).emissive->Bind(currentBuffer, &graphicsPipeline.GetLayout(), 4);
				it.texture.at(i).normalMap->Bind(currentBuffer, &graphicsPipeline.GetLayout(), 5);
				currentBuffer->drawIndexed(it.mesh->GetSubMesh(i).IndexCount(), 1, 0, 0, 0);
			}
		}
	}
	//currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *linePipeLine.Get());
	//currentBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
	//currentBuffer->bindVertexBuffers(0, *vertexBuffer.Get(), { 0 });
	//currentBuffer->bindIndexBuffer(*indexBuffer.Get(), 0, vk::IndexType::eUint32);

	//currentBuffer->drawIndexed(36, 1, 0, 0, 0);

	currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *uiPipeline.Get());

	for (auto& ui : uIRender)
	{
		// bind the vertex and indexBuffer
		currentBuffer->bindVertexBuffers(0, *uiVertexBuffer.Get(), {0});
		currentBuffer->bindIndexBuffer(*uiIndexBuffer.Get(), 0, vk::IndexType::eUint32);
		currentBuffer->pushConstants<UiData::UiValidData>(uiPipeline.GetLayout(), vk::ShaderStageFlagBits::eVertex, 0,ui.second);
		ui.first->Bind(currentBuffer, &uiPipeline.GetLayout(), 0);
		currentBuffer->drawIndexed(static_cast<std::uint32_t>(uiIndexBuffer.GetSize() / sizeof(std::uint32_t)), 1, 0, 0, 0);
		currentBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *linePipeLine.Get());
		currentBuffer->bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, graphicsPipeline.GetLayout(), 0, *descriptorSets.Get(), nullptr);
		currentBuffer->bindVertexBuffers(0, *vertexBuffer.Get(), { 0 });
		currentBuffer->bindIndexBuffer(*indexBuffer.Get(), 0, vk::IndexType::eUint32);
		currentBuffer->drawIndexed(36, 1, 0, 0, 0);
	}

	EndRendering(window, currentBuffer, { syncObject.GetCurrentPresentSemaphore() }, imguiDraw);
	commandBuffers.ReleaseCommandBuffer(*currentBuffer);
	syncObject.IncrementFrame();
	device.Get().waitIdle();

	m_ubo.reset();
	m_toRenderObject.clear();
	m_lights.clear(); 
	uIRender.clear();
	m_frameBones.clear();
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

KGR::_Vulkan::DebugRenderer& KGR::_Vulkan::VulkanCore::GetDebugRenderer()
{
	return debugRenderer;
}

const KGR::_Vulkan::DebugRenderer& KGR::_Vulkan::VulkanCore::GetDebugRenderer() const
{
	return debugRenderer;
}



//IMPL