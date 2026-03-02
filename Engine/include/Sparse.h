#pragma once

#include <vector>
#include <ranges>
#include <algorithm>
#include <stdexcept>

/**
 * @brief Sparse set–style storage for arithmetic entity types.
 *
 * This container implements a classic sparse set structure:
 * - `m_entities` stores all active entities densely.
 * - `m_sparse` maps an entity value to its index in `m_entities`.
 *
 * It provides O(1) insertion, removal, lookup, and index retrieval.
 *
 * @tparam Type   Entity type (must be arithmetic).
 * @tparam offset Growth offset used when resizing the sparse array.
 */
template<typename Type,size_t offset = 100> requires (std::is_arithmetic_v<Type>)
struct Sparse_Storage
{
	using type = Type;
	using signed_type = std::make_signed_t<Type>;

	/**
	 * @brief the invalid value for the sparse
	 */
	static constexpr signed_type invalidValue = static_cast<signed_type>(-1);

	Sparse_Storage() = default;
	/**
	 * 
	 * @param size the starting size of the sparse to avoid multiple reserve
	 */
	Sparse_Storage(type size);

	/**
	 * @brief this function verify if the entity is in the sparse 
	 * @param e the entity
	 * @return boolean
	 */
	bool Has(const type& e) const;

	/**
	 * @brief this function add an entity and throw if already added
	 * @param e the entity
	 * 
	 */
	void Add(const type& e);

	/**
	 * @brief this function remove an entity and throw if not added
	 * @param e the entity
	 */
	void Remove(const type& e);

	/**
	 * @brief this function return all the entities
	 * @return const vector of type& 
	 */
	const std::vector<type> GetEntities() const;

	/**
	 * @brief this function return the size of the entity vector
	 * @return size_t
	 */
	size_t Size() const;

	/**
	 * @brief return the sparse index for an entity and throw if not added
	 * @param e the entity
 	 * @return signed type of entity
	 */
	signed_type GetIndex(const type& e) const;

	/**
	 * @brief get the size of the sparse size 
	 * @return type of entity
	 */
	type AvailableSize() const;
private:
	/**
	 * @brief this function resize an
	 * @param index index 
	 */
	void Resize(const type& index);

	std::vector<type> m_entities;
	std::vector<signed_type> m_sparse;
};

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
Sparse_Storage<Type, offset>::Sparse_Storage(type size)
{
	m_sparse.resize(size);
	std::ranges::fill(m_sparse,invalidValue);
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
bool Sparse_Storage<Type, offset>::Has(const type& e) const
{
	return m_sparse.size() > e && m_sparse[e] != invalidValue;
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
void Sparse_Storage<Type, offset>::Add(const type& e)
{
	if (Has(e))
		throw std::out_of_range("entity already store");

	if (m_sparse.size() <= e)
		Resize(e);

	m_sparse[e] = static_cast<signed_type>(m_entities.size());
	m_entities.push_back(e);
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
void Sparse_Storage<Type, offset>::Remove(const type& e)
{
	if (!Has(e))
		throw std::out_of_range("entity not stored");

	signed_type index = m_sparse[e];
	type lastEntity = std::move(m_entities.back());

	m_entities[index] = lastEntity;
	m_entities.pop_back();

	m_sparse[lastEntity] = index;
	m_sparse[e] = invalidValue;
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
const std::vector<typename Sparse_Storage<Type, offset>::type> Sparse_Storage<Type, offset>::GetEntities() const
{
	return m_entities;
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
typename size_t Sparse_Storage<Type, offset>::Size() const
{
	return m_entities.size();
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
typename Sparse_Storage<Type, offset>::signed_type Sparse_Storage<Type, offset>::GetIndex(const type& e) const
{
	if (!Has(e))
		throw std::out_of_range("entity not store");
	return m_sparse[e];
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
typename Sparse_Storage<Type, offset>::type Sparse_Storage<Type, offset>::AvailableSize() const
{
	return m_sparse.size();
}

template <typename Type, size_t offset> requires (std::is_arithmetic_v<Type>)
void Sparse_Storage<Type, offset>::Resize(const type& index)
{
	size_t lastSize = m_sparse.size();
	m_sparse.resize(lastSize + index + offset);
	std::ranges::fill(m_sparse | std::views::drop(lastSize) | std::views::take(index + offset), invalidValue);
}
