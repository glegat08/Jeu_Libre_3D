#pragma once

#include <type_traits>
#include <vector>

namespace KGR
{
	namespace ECS
	{
		/**
		 * 
		 * @brief forward declaration of the registry
		 * @tparam Type the type entity for the sparse must be arithmetic
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset > requires (std::is_arithmetic_v<Type>)
		struct Registry;

		/**
		 * @brief a view is a container of entity that match the type requirement only created by the registry
		 * @tparam Type the type entity for the sparse must be arithmetic
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset = 100> requires (std::is_arithmetic_v<Type>)
		class View
		{
		public:
			using type = Type;
			using iterator = typename std::vector<type>::iterator;
			using const_iterator = typename  std::vector<type>::const_iterator;
			friend Registry<Type, offset>;

			/**
			 * @brief return all the entities
			 * @return vector of entity
			 */
			const std::vector<type>& GetEntities() const;
			/**
			 * @brief return the size of the entity vector
			 * @return size_t
			 */
			size_t Size() const;
			/**
			 * 
			 * @return 
			 */
			 /**
			  * @brief return the first iterator for range-based loop
			  * @return iterator
			  */
			iterator begin();

			/**
			 * @brief return the first iterator const for range-based loop
			 * @return const iterator
			 */
			 const_iterator begin() const;

			/**
			 * @brief return the last + 1 iterator for range-based loop
			 * @return iterator
			 */
			iterator end();

			/**
			 * @brief return the last + 1 iterator const for range-based loop
			 * @return const iterator
			 */
			 const_iterator end() const;

		private:
			void AddEntity(const type& t);
			std::vector<type> m_entities;
		};

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		const std::vector<typename View<Type, offset>::type>& View<Type, offset>::GetEntities() const
		{
			return m_entities;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		size_t View<Type, offset>::Size() const
		{
			return m_entities.size();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename View<Type, offset>::iterator View<Type, offset>::begin()
		{
			return m_entities.begin();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename View<Type, offset>::iterator View<Type, offset>::end()
		{
			return m_entities.end();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename View<Type, offset>::const_iterator View<Type, offset>::begin() const
		{
			return m_entities.begin();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename View<Type, offset>::const_iterator View<Type, offset>::end() const
		{
			return m_entities.end();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		void View<Type, offset>::AddEntity(const type& t)
		{
			m_entities.push_back(t);
		}
	}
}
