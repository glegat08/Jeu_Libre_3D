#pragma once
#include <atomic>
#include <type_traits>
#include "Sparse.h"
/**
 * @brief Concept verifying a Component.
 * 
 *  do not have custom constructor 
 */
template<typename Type>
concept CompType = std::is_trivial_v<Type>;

namespace KGR
{
	namespace ECS
	{
		/**
		 * @brief a custom rtti for components
		 */
		struct CompId
		{
		public:
			/**
			 * @brief get the unique type id of a component
			 * @tparam Component the component you want to verify
			 * @return size_t
			 */
			template<CompType Component>
			static size_t GetId();
		private:
			static std::atomic_size_t m_id;
		};
		inline std::atomic_size_t CompId::m_id = 0;

		template <CompType Component>
		size_t CompId::GetId()
		{
			static size_t id = m_id++;
			return id;
		}

		/**
		 * @brief the base of component container must have an implementation that use sparse to optimise
		 * @tparam Type the type entity for the sparse must be arithmetic 
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset = 100> requires std::is_arithmetic_v<Type>
		struct Component_Container_Base
		{
			using type = Type;
			using storage = Sparse_Storage<Type, offset>;
			/**
			 * @brief virtual destructor
			 */
			virtual ~Component_Container_Base() = default;

			/**
			 * @brief verify if this container have the component for this entity
			 * @param e the entity
			 * @return boolean
			 */
			bool HasComponent(const type& e) const;

			/**
			 * @brief remove the component of an entity throw if not added
			 * @param e the entity
			 */
			virtual void Remove(const type& e) = 0;
		protected:
			storage m_storage;
		};

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		bool Component_Container_Base<Type, offset>::HasComponent(const type& e) const
		{
			return m_storage.Has(e);
		}

		/**
		 * @brief  implementation of the container base that handle one type of component
		 * @tparam Component the type of component stored
		 * @tparam Type same as container base
		 * @tparam offset same as container base
		 */
		template<CompType Component, typename Type, size_t offset = 100> requires std::is_arithmetic_v<Type>
		struct Component_Container : public Component_Container_Base<Type, offset>
		{
			using type = Type;

			/**
			 * @brief this function add a component by calling the sparse add 
			 * @param e the entity
			 */
			void AddComponent(const type& e);

			/**
			 * @brief this function add a component with an r value of the component by calling the sparse add
			 * @param e the entity
			 * @param c r value to component
			 */
			void AddComponent(const type& e,Component&& c);

			/**
			 * @brief this function remove a component by calling the sparse remove 
			 * @param e the entity
			 */
			void Remove(const type& e) override;

			/**
			 * @brief this function return the right component by calling the sparse GetIndex 
			 * @param e the entity
			 * @return the &Component link to the entity
			 */
			Component& GetComponent(const type& e);

			/**
			 * @brief this function is the const version 
			 * @param e the entity
			 * @return the &Component link to the entity
			 */
			const Component& GetComponent(const type& e) const;

		private:
			std::vector<Component> m_components;

		};

		template <CompType Component, typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		void Component_Container<Component, Type, offset>::AddComponent(const type& e)
		{
			this->m_storage.Add(e);
			m_components.push_back(Component{});
		}

		template <CompType Component, typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		void Component_Container<Component, Type, offset>::AddComponent(const type& e, Component&& c)
		{
			this->m_storage.Add(e);
			m_components.push_back(std::move(c));
		}

		template <CompType Component, typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		void Component_Container<Component, Type, offset>::Remove(const type& e)
		{
			auto index = this->m_storage.GetIndex(e);
			this->m_storage.Remove(e);
			m_components[index] = std::move(m_components.back());
			m_components.pop_back();
		}

		template <CompType Component, typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		Component& Component_Container<Component, Type, offset>::GetComponent(const type& e)
		{
			return m_components[this->m_storage.GetIndex(e)];
		}

		template <CompType Component, typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		const Component& Component_Container<Component, Type, offset>::GetComponent(const type& e) const
		{
			return m_components[this->m_storage.GetIndex(e)];
		}
	}
}
