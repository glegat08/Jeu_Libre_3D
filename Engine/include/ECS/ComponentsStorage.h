#pragma once
#include <type_traits>
#include <memory>
#include "Sparse.h"
#include "Component.h"


namespace KGR
{
	namespace ECS
	{
		/**
		 * @brief the class that contain all the component container
		 * @tparam Type the type entity for the sparse must be arithmetic
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
			struct Components_Container_Storage
		{
			using type = Type;
			using storage = Sparse_Storage<Type, offset>;
			using base = Component_Container_Base<Type, offset>;
			template<CompType Component>
			using derived = Component_Container<Component, Type, offset>;

			/**
			 * @brief if the class has the component storage by calling the sparse has
			 * @tparam Component the type of component
			 * @return boolean
			 */
			template<CompType Component>
			bool Has() const;

			/**
			 * @brief add a component storage by calling the sparse add
			 * @tparam Component the type of component
			 */
			template<typename Component>
			void Add();

			/**
			 * @brief remove a component storage by calling the sparse remove
			 * @tparam Component the type of component
			 */
			template<typename Component>
			void Remove();

			/**
			 * @brief return the component storage throw if not added
			 * @tparam Component the type of component
			 * @return a &Component storage of the wanted type
			 */
			template<typename Component>
			derived<Component>& Get() const ;

			/**
			 * @brief return the component storage add if missing
			* @tparam Component the type of component
			* @return a &Component storage of the wanted type
			*/
			template<typename Component>
			derived<Component>& GetAndAdd();

			/**
			 * 
			 * @return return all the Component container that are stored in
			 */
			const std::vector<std::unique_ptr<base>>& GetPools() const;

		private:
			std::vector<std::unique_ptr<base>> m_pools;
			storage m_storage;
		};

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		bool Components_Container_Storage<Type, offset>::Has() const
		{
			return m_storage.Has(CompId::GetId<Component>());
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <typename Component>
		void Components_Container_Storage<Type, offset>::Add()
		{
			m_storage.Add(CompId::GetId<Component>());
			m_pools.push_back(std::make_unique<derived<Component>>());
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <typename Component>
		void Components_Container_Storage<Type, offset>::Remove()
		{
			type index = m_storage.GetIndex(CompId::GetId<Component>());
			m_storage.Remove(CompId::GetId<Component>());
			m_pools[index] = std::move(m_pools.back());
			m_pools.pop_back();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <typename Component>
		typename Components_Container_Storage<Type, offset>::template derived<Component>& Components_Container_Storage<Type,
		offset>::Get() const
		{
			if (!m_storage.Has(CompId::GetId<Component>()))
				throw std::out_of_range("entity not store");
			return static_cast<derived<Component>&>(*m_pools[m_storage.GetIndex(CompId::GetId<Component>())].get());
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <typename Component>
		typename Components_Container_Storage<Type, offset>::template derived<Component>& Components_Container_Storage<Type,
		offset>::GetAndAdd()
		{
			if (!Has<Component>())
				Add<Component>();
			return Get<Component>();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		const std::vector<std::unique_ptr<typename Components_Container_Storage<Type, offset>::base>>&
		Components_Container_Storage<Type, offset>::GetPools() const
		{
			return m_pools;
		}
	}
}