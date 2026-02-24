#pragma once
#include <type_traits>
#include "ComponentsStorage.h"
#include "EntityPool.h"
#include "Filtre.h"
#include "View.h"

namespace KGR
{
	namespace ECS
	{
		/**
		 * @brief the major class of rtti that contain all the entity and all the component pools
		 * @tparam Type   type of entity (must be arithmetic)
         * @tparam offset Growth offset when resizing
		 */
		template<typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
			struct Registry
		{
			using type = Type;
			/**
			 * @brief return an available entity
			 * @return entity
			 */
			type CreateEntity();

			/**
			 * 
			 * @param e the entity
			 */
			void DestroyEntity(const type& e);

			/**
			 * @brief add a component to an entity
			 * @tparam Component the type of component
			 * @param e the entity
			 */
			template<CompType Component>
			void AddComponent(const type& e);

			/**
			 * @brief add multiple component to an entity
			 * @tparam Components variadic of type component 
			 * @param e the entity
			 */
			template<CompType... Components>
			void AddComponents(const type& e);

			/**
			 * @brief add a component to an entity with value
			 * @tparam Component the type of component
			 * @param e the entity
			 * @param c r value to the component
			 */
			template<CompType Component>
			void AddComponent(const type& e, Component&& c);

			/**
			 * 
			 * @brief add multiple component to an entity with values
			 * @tparam Components variadic of type component
			 * @param e the entity
			 * @param c r value to the components
			 */
			template<CompType... Components>
			void AddComponents(const type& e,Components&&... c);

			/**
			 * @brief remove a component to an entity
			 * @tparam Component the type of component
			 * @param e the entity
			 */
			template<CompType Component>
			void RemoveComponent(const type& e);

			/**
			 * @brief remove multiple components to an entity
			 * @tparam Components the type of components
			 * @param e the entity
			 */
			template<CompType... Components>
			void RemoveComponents(const type& e);

			/**
			 * @brief verify if the component is stored and if the entity has it 
			 * 
			 * @tparam Component the type of component
			 * @param e the entity
			 * @return boolean
			 */
			template<CompType Component>
			bool HasComponent(const type& e) const;

			/**
			 * @brief verify if the  all the components is stored and if the entity have all
			 * @tparam Components the types of components 
			 * @param e the entity
			 * @return boolean
			 */
			template<CompType... Components>
			bool HasAllComponents(const type& e) const;


			/**
			 * @brief verify if the entity have at least one component of the list
			 * @tparam Components the types of components
			 * @param e the entity
			 * @return boolean
			 */
			template<CompType... Components>
			bool HasAnyComponents(const type& e) const ;

			/**
			 * @brief return the component of an entity
			 * @tparam Component the component type
			 * @param e the entity
			 * @return &component
			 */
			template<CompType Component>
			Component& GetComponent(const type& e);

			/**
			 * @brief return all the valid entities
			 * @return vector of entity
			 */
			const std::vector<type>& GetAllEntities() const;

			/**
			 * @brief create a filter of all entities that has all the components
			 * @tparam Components the types of components
			 * @return a filter of multiple component
			 */
			template<CompType... Components>
			Filter<Type,offset> GetAllComponentsFilter();

			/**
			* @brief create a filter of all entities that has any of the components
			* @tparam Components the types of components
			* @return a filter of multiple component
			*/
			template<CompType... Components>
			Filter<Type, offset> GetAnyComponentsFilter();

			/**
			 * @brief create a filter of all entities that has this component
			 * @tparam Component the type of component
			 * @return a filter with single component
			 */
			template<CompType Component>
			Filter<Type, offset> GetFilter();

			/**
			 * @brief create a view of all the entity that contained this component
			 * @tparam Component the type of component
			 * @return a view for a single type of component
			 */
			template<CompType Component>
			View<Type,offset> GetView();

			/**
			 * @brief create a view of all the entity that contained all the components
			 * @tparam Components the types of components
			 * @return a filter with multiple components
			 */
			template<CompType... Components>
			View<Type, offset> GetAllComponentsView();

			/**
			 * @brief create a view of all the entity that contained any of the components
			 * @tparam Components the types of components
			 * @return a filter with multiple components
			 */
			template<CompType... Components>
			View<Type, offset> GetAnyComponentsView();

		private:
			/**
			 * @brief private function that verify that the entity is valid 
			 * @param e the entity
			 * @return boolean
			 */
			bool HasEntity(const type& e) const;
			Components_Container_Storage<Type, offset> m_pool;
			EntityPool<Type, offset * 10> m_entityPool;
		};


		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		typename Registry<Type, offset>::type Registry<Type, offset>::CreateEntity()
		{
			return m_entityPool.CreateEntity();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		void Registry<Type, offset>::DestroyEntity(const type& e)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			m_entityPool.RemoveEntity(e);
			auto& pool = m_pool.GetPools();
			for (auto& it : pool)
			{
				if (it->HasComponent(e))
					it->Remove(e);
			}
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		void Registry<Type, offset>::AddComponent(const type& e)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");
			m_pool.template GetAndAdd<Component>().AddComponent(e);
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		void Registry<Type, offset>::AddComponent(const type& e, Component&& c)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");
			m_pool.template GetAndAdd<Component>().AddComponent(e, std::move(c));
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		void Registry<Type, offset>::RemoveComponent(const type& e)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			m_pool.template Get<Component>().Remove(e);
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		bool Registry<Type, offset>::HasComponent(const type& e) const 
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			if ( !m_pool.template Has<Component>())
				return false;

			if ( !m_pool.template Get<Component>().HasComponent(e))
				return false;

			return true;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		bool Registry<Type, offset>::HasAllComponents(const type& e) const
		{
			{
				if (!HasEntity(e))
					throw std::out_of_range("entity not stored");

				if ((... || !m_pool.template Has<Components>()))
					return false;

				if ((... || !m_pool.template Get<Components>().HasComponent(e)))
					return false;

				return true;
			}
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		View<Type, offset> Registry<Type, offset>::GetView()
		{
			View<Type, offset> view;
			for (auto& e : m_entityPool.GetEntities())
			{
				if (HasComponent<Component>(e))
					view.AddEntity(e);
			}
			return view;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		View<Type, offset> Registry<Type, offset>::GetAllComponentsView()
		{
			View<Type, offset> view;
			for (auto& e : m_entityPool.GetEntities())
			{
				if (HasAllComponents<Components...>(e))
					view.AddEntity(e);
			}
			return view;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		View<Type, offset> Registry<Type, offset>::GetAnyComponentsView()
		{
			View<Type, offset> view;
			for (auto& e : m_entityPool.GetEntities())
			{
				if (HasAnyComponents<Components...>(e))
					view.AddEntity(e);
			}
			return view;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		bool Registry<Type, offset>::HasAnyComponents(const type& e) const 
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			if ((... || (m_pool.template Has<Components>() == true ? m_pool.template Get<Components>().HasComponent(e) : false)))
				return true;

			return false;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		Filter<Type, offset> Registry<Type, offset>::GetAnyComponentsFilter()
		{
			Filter<Type, offset> filter(m_entityPool.Size());
			(..., (filter.template Add<Components>(&m_pool.template GetAndAdd<Components>())));
			for (auto& e : m_entityPool.GetEntities())
			{
				if (HasAnyComponents<Components...>())
					filter.AddEntity(e);
			}
			return filter;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		Filter<Type, offset> Registry<Type, offset>::GetFilter()
		{
			Filter<Type, offset> filter(m_entityPool.Size());
			filter.template Add<Component>(&m_pool.template GetAndAdd<Component>());
			for (auto& e : m_entityPool.GetEntities())
			{
				if (HasComponent<Component>(e))
					filter.AddEntity(e);
			}
			return filter;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType... Components>
		void Registry<Type, offset>::AddComponents(const type& e)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			(..., m_pool.template GetAndAdd<Components>().AddComponent(e));
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		void Registry<Type, offset>::AddComponents(const type& e, Components&&... c)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");
			(..., m_pool.template GetAndAdd<Components>().AddComponent(e, std::move(c)));
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType... Components>
		void Registry<Type, offset>::RemoveComponents(const type& e)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			(..., m_pool.template Get<Components>().Remove(e));
			
		}


		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType Component>
		Component& Registry<Type, offset>::GetComponent(const type& e)
		{
			if (!HasEntity(e))
				throw std::out_of_range("entity not stored");

			auto& component = m_pool.template Get<Component>();
			return component.GetComponent(e);
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		const std::vector<typename Registry<Type, offset>::type>& Registry<Type, offset>::GetAllEntities() const
		{
			return m_entityPool.GetEntities();
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		template <CompType ... Components>
		Filter<Type, offset> Registry<Type, offset>::GetAllComponentsFilter()
		{
			Filter<Type, offset> filter(m_entityPool.Size());
			(..., (filter.template Add<Components>( &m_pool.template Get<Components>() ) ) );
			for (auto& e : m_entityPool.GetEntities())
			{
				if (HasAllComponents<Components...>(e))
					filter.AddEntity(e);
			}
			return filter;
		}

		template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
		bool Registry<Type, offset>::HasEntity(const type& e) const
		{
			return m_entityPool.HasEntity(e);
		}
	}
}
