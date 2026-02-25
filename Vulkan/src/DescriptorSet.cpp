#include "DescriptorSet.h"
#include "Device.h"
#include "DescriptorLayout.h"
#include "DescriptorPool.h"

KGR::_Vulkan::DescriptorSet::DescriptorSet(Device* device, DescriptorPool* pool,
	DescriptorLayout* layout)
{
	vk::DescriptorSetAllocateInfo        allocInfo{
		   .descriptorPool = pool->Get(),
		   .descriptorSetCount = 1,
		   .pSetLayouts = &(*layout->Get()) };
	m_set =std::move(device->Get().allocateDescriptorSets(allocInfo).front());
}

std::vector<KGR::_Vulkan::DescriptorSet> KGR::_Vulkan::DescriptorSet::Create(Device* device, DescriptorPool* pool, DescriptorLayout* layout, size_t count)
{
	std::vector<vk::DescriptorSetLayout> layouts(count, layout->Get());

	vk::DescriptorSetAllocateInfo        allocInfo{
		   .descriptorPool = pool->Get(),
		   .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		   .pSetLayouts = layouts.data() };
	auto sets = device->Get().allocateDescriptorSets(allocInfo);
	std::vector<KGR::_Vulkan::DescriptorSet> result;
	for (size_t i = 0 ; i < count ; ++i)
	{
		result.push_back(std::move(sets.back()));
		sets.pop_back();
	}
	return result;
}
