#pragma once
#include <type_traits>
#include "Sparse.h"
#include "Component.h"
#include <memory>
namespace KGR
{
	namespace ECS
	{
		/**
		 * @brief forward declaration of the registry
		 * @tparam Type the type entity for the sparse must be arithmetic
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset > requires (std::is_arithmetic_v<Type>)
			struct Registry;

		/**
		 * @brief the filter is a claas that has entity and Components
		 * @tparam Type the type entity for the sparse must be arithmetic
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
			struct Filter
		{
			friend Registry<Type, offset>;
			using type = Type;
			using storage = Sparse_Storage<Type, offset>;
			using base = Component_Container_Base<Type, offset>;
			template<CompType Component>
			using derived = Component_Container<Component, Type, offset>;
			using iterator = std::vector<type>::iterator;
			using const_iterator = std::vector<type>::const_iterator;

			/**
			 * @brief return all the entity stored in the filter
			 * @return vector of entity
			 */
			const std::vector<type> GetEntities() const;

			/**
			 * @brief return the component of an entity by call the sparse get
			 * @tparam Component the type of component
			 * @param e the entity
			 * @return a &Component
			 */
			template<CompType Component>
			Component& GetComponent(const type& e);

			/**
			 * @brief return the vector size of entity
			 * @return size_t 
			 */
			size_t Size() const;

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
			/**
			 * @brief the private constructor for registry 
			 * @param size size_t to avoid multiple allocation
			 */
			Filter(size_t size);

			/**
			 * @brief private function to add a component storage by the registry by call the sparse add
			 * @tparam Component the type of component
			 * @param ptr a pointer to the component storage
			 */
			template<CompType Component>
			void Add(derived<Component>* ptr);

			/**
			 * @brief add an entity
			 * @param e tye entity
			 */
			void AddEntity(const type& e);



			std::vector<type> m_entities;
			std::vector<base*> m_pools;
			storage m_storage;
		};


		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		const std::vector<typename Filter<Type, offset>::type> Filter<Type, offset>::GetEntities() const
		{
			return m_entities;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		Component& Filter<Type, offset>::GetComponent(const type& e)
		{
			return static_cast<derived<Component>&>(*m_pools[m_storage.GetIndex(CompId::GetId<Component>())]).GetComponent(e);
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename size_t Filter<Type, offset>::Size() const
		{
			return m_entities.size();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename Filter<Type, offset>::iterator Filter<Type, offset>::begin()
		{
			return m_entities.begin();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		 typename Filter<Type, offset>::const_iterator Filter<Type, offset>::begin() const
		{
			return m_entities.begin();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename Filter<Type, offset>::iterator Filter<Type, offset>::end()
		{
			return m_entities.end();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		 typename Filter<Type, offset>::const_iterator Filter<Type, offset>::end() const
		{
			return m_entities.end();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		Filter<Type, offset>::Filter(size_t size)
		{
			m_entities.reserve(size);
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		void Filter<Type, offset>::Add(derived<Component>* ptr)
		{
			m_storage.Add(CompId::GetId<Component>());
			m_pools.push_back(ptr);
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		void Filter<Type, offset>::AddEntity(const type& e)
		{
			m_entities.push_back(e);
		}
	}
}
