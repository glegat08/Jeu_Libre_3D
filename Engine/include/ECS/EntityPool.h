#pragma once
#include <type_traits>
#include "Sparse.h"

namespace KGR
{
	namespace ECS
	{
		/**
		 * @brief this is the entity pool that recycling the entity 
		 * @tparam Type the type entity for the sparse must be arithmetic
		 * @tparam offset same as sparse parameter
		 */
		template<typename Type, size_t offset = 1000> requires std::is_arithmetic_v<Type>
		struct EntityPool
		{
		public:
			using type = Type;
			using storage = Sparse_Storage<Type, offset>;

			EntityPool();
			/**
			 * @brief this function verify if the entity is stored by call the sparse has
			 * @param e the entity
			 * @return a boolean
			 */
			bool HasEntity(const type& e) const;

			/**
			 * @brief create an entity with valid id by call the sparse add
			 * @return the entity
			 */
			type CreateEntity();

			/**
			 * @brief remove an entity by call the sparse remove 
			 * @param e the entity
			 */
			void RemoveEntity(const type& e);

			/**
			 * @brief return all the valid entity by call the sparse getEntities
			 * @return vector en entity
			 */
			const std::vector<type> GetEntities() const ;

			/**
			 * @brief the size of the entity vector 
			 * @return size_t
			 */
			size_t Size() const;

		private:
			/**
			 * @brief fill with valid id
			 */
			void FillWithId();
			type m_lastEntityCreated;
			std::vector<type> m_freeEntities;
			storage m_storage;
		};

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		EntityPool<Type, offset>::EntityPool() : m_lastEntityCreated(static_cast<type>(0))
		{
		}

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		bool EntityPool<Type, offset>::HasEntity(const type& e) const
		{
			return m_storage.Has(e);
		}

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		typename EntityPool<Type, offset>::type EntityPool<Type, offset>::CreateEntity()
		{
			if (m_freeEntities.empty())
				FillWithId();
			auto id = m_freeEntities.back();
			m_freeEntities.pop_back();
			m_storage.Add(id);
			return id;
		}

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		void EntityPool<Type, offset>::RemoveEntity(const type& e)
		{
			type index = m_storage.GetIndex(e);
			m_storage.Remove(e);
			m_freeEntities.push_back(e);
		}

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		const std::vector<typename EntityPool<Type, offset>::type> EntityPool<Type, offset>::GetEntities() const 
		{
			return m_storage.GetEntities();
		}

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		typename size_t EntityPool<Type, offset>::Size() const
		{
			return m_storage.Size();
		}

		template <typename Type, size_t offset> requires std::is_arithmetic_v<Type>
		void EntityPool<Type, offset>::FillWithId()
		{
			m_freeEntities.resize(offset);
			m_lastEntityCreated  += offset - 1;
			type currentEntity = m_lastEntityCreated;
			for (size_t i = 0; i < offset; ++i)
			{
				m_freeEntities[i] = currentEntity--;
			}
		}

	}
}