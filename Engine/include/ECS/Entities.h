#pragma once
#include <cstdint>
namespace KGR
{
	namespace ECS
	{
		/**
		 * @brief base alias for entity but not restricted you can use your own types 
		 */
		struct Entity
		{
			using _64 = std::uint64_t;
			using _32 = std::int32_t;
			using _16 = std::uint16_t;
			using _8 = std::uint8_t;
		};
		
	}
}	