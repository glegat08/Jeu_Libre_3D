#include "Pipeline.h"

#include "DescriptorLayout.h"
#include "Device.h"
#include "SwapChain.h"
#include "VulkanCore.h"
#include "Core/ManagerImple.h"
#include "PhysicalDevice.h"

KGR::_Vulkan::Pipeline::Pipeline(const ShaderInfo& shaderInfo, Device* device, SwapChain* swapChain, DescriptorLayouts* layouts, PhysicalDevice* phDevice, vk::PolygonMode mode, const vk::VertexInputBindingDescription& vInput, const std::vector < vk::VertexInputAttributeDescription>& attributes)
{

	//
	auto& file = FileManager::Load(shaderInfo.ShaderPath);
	file.seekg(0, std::ios::end);
	auto fileSize = file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	file.close();
	FileManager::Unload(shaderInfo.ShaderPath);

	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = buffer.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(buffer.data()) };
	vk::raii::ShaderModule shaderModule{ device->Get(), createInfo };

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = shaderInfo.vertexMain };
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = shaderInfo.fragmentMain };
	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	auto                                     bindingDescription =vInput;
	auto                                     attributeDescriptions = attributes;
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data() };
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
		.topology = topology,
		.primitiveRestartEnable = vk::False };
	vk::PipelineViewportStateCreateInfo viewportState{
		.viewportCount = 1,
		.scissorCount = 1 };
	vk::PipelineRasterizationStateCreateInfo rasterizer{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = mode,
		.cullMode = cullMode,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f };
	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False };
	vk::PipelineDepthStencilStateCreateInfo depthStencil{
		.depthTestEnable = vk::True,
		.depthWriteEnable = vk::True,
		.depthCompareOp = vk::CompareOp::eLess,
		.depthBoundsTestEnable = vk::False,
		.stencilTestEnable = vk::False };
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };
	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment };
	std::vector dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data() };

	auto activeLayouts = layouts->GetLayouts();
	for (auto& a: activeLayouts)
	{
		m_layouts.push_back(*a);
	}

	vk::PushConstantRange pushRange{
	vk::ShaderStageFlagBits::eVertex, 
	0,                                
	80                 
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount = static_cast<uint32_t>(layouts->GetLayouts().size()),
		.pSetLayouts = m_layouts.data(),
		
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushRange };

	m_layout = vk::raii::PipelineLayout(device->Get(), pipelineLayoutInfo);

	vk::Format depthFormat = phDevice->findSupportedFormat(
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
			{.stageCount = 2,
			 .pStages = shaderStages,
			 .pVertexInputState = &vertexInputInfo,
			 .pInputAssemblyState = &inputAssembly,
			 .pViewportState = &viewportState,
			 .pRasterizationState = &rasterizer,
			 .pMultisampleState = &multisampling,
			 .pDepthStencilState = &depthStencil,
			 .pColorBlendState = &colorBlending,
			 .pDynamicState = &dynamicState,
			 .layout = m_layout,
			 .renderPass = nullptr},
			{.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChain->GetFormat().format, .depthAttachmentFormat = depthFormat} };

	m_pipeline = vk::raii::Pipeline(device->Get(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

KGR::_Vulkan::Pipeline::vkPipelineLayout& KGR::_Vulkan::Pipeline::GetLayout()
{
	return m_layout;
}

const KGR::_Vulkan::Pipeline::vkPipelineLayout& KGR::_Vulkan::Pipeline::GetLayout() const
{
	return m_layout;
}

KGR::_Vulkan::Pipeline::vkPipeline& KGR::_Vulkan::Pipeline::Get()
{
	return m_pipeline;
}

const KGR::_Vulkan::Pipeline::vkPipeline& KGR::_Vulkan::Pipeline::Get() const
{
	return m_pipeline;
}