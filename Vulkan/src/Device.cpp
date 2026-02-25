#include "Device.h"
#include "PhysicalDevice.h"
#include "Surface.h"
KGR::_Vulkan::Device::Device(PhysicalDevice* device, Surface* surface, ui32t count)
{

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = device->Get().getQueueFamilyProperties();
	auto graphicsQueueFamilyProperty = std::find_if(
		queueFamilyProperties.begin(),
		queueFamilyProperties.end(),
		[](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; }
	);
	 m_queueIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	 m_queueCount = count;

	float queuePriority = 0.0f;
	auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo
	{
		.queueFamilyIndex = m_queueIndex,
		.queueCount = count,
		.pQueuePriorities = &queuePriority
	};

	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
		{.features = {.fillModeNonSolid = true,.samplerAnisotropy = true } },
		{.shaderDrawParameters = true },
		{.bufferDeviceAddress = true },
		{.synchronization2 = true, .dynamicRendering = true },{}
	};

	std::vector<const char*> deviceExtensions =
	{
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName
	};

	vk::DeviceCreateInfo deviceCreateInfo
	{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = count,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data()
	};

	m_device = vkDevice(device->Get(), deviceCreateInfo);
}

KGR::_Vulkan::Device::vkDevice& KGR::_Vulkan::Device::Get()
{
	return m_device;
}

const KGR::_Vulkan::Device::vkDevice& KGR::_Vulkan::Device::Get() const
{
	return m_device;
}
