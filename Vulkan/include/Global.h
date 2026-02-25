#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN        // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>



const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

#include"vma/vk_mem_alloc.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <tiny_obj_loader.h>
#include <glm/gtx/hash.hpp>

using ui32t = uint32_t;
using i32t = int32_t;




//const std::vector<Vertex> vertices2 = {
//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f,1.0f}, {1.0f, 0.0f}},
//	{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f,1.0f}, {0.0f, 0.0f}},
//	{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f,1.0f}, {0.0f, 1.0f}},
//	{{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f,1.0f}, {1.0f, 1.0f}},
//
//	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f,1.0f}, {1.0f, 0.0f}},
//	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f,1.0f}, {0.0f, 0.0f}},
//	{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f,1.0f}, {0.0f, 1.0f}},
//	{{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f,1.0f}, {1.0f, 1.0f}} };
//
//const std::vector<uint16_t> indices2 = {
//	// top
//	2, 1, 0,
//	0, 3, 2,
//
//	// forward
//	7, 6, 2,
//	2, 3, 7,
//
//	// right
//	6, 5, 1,
//	1, 2, 6,
//
//	// left
//	4, 7, 3,
//	3, 0, 4,
//
//	// backward
//	5, 4, 0,
//	0, 1, 5,
//
//	// down
//	4, 5, 6,
//	6, 7, 4
//};

namespace KGR
 {
	namespace _Vulkan
	{
		inline static vk::raii::Context vkContext;

	}
}


struct UniformBufferObject {
	glm::mat4 transform;
	glm::mat4 view;
	glm::mat4 proj;
};