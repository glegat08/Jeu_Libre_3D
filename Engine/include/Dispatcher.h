#pragma once
#include <map>
#include <string>
#include <functional>
#include <stdexcept>

/**
 * @brief Dispatcher system mapping pairs of dynamic types to functions.
 *
 * This module provides a type‑based dispatch mechanism similar to
 * double‑dispatch or multimethods. Functions are registered using pairs of
 * derived types and later invoked based on the dynamic types of the objects.
 */
namespace KGR
{
	/**
	 * @brief Wrapper struct used to store a variadic list of argument types.
	 *
	 * This type is only used as a template tag to pass a list of argument
	 * types to Dispatcher and FNDispatcher.
	 *
	 * @tparam Args Variadic list of argument types.
	 */
	template<typename... Args>
	struct WhrapperTypes
	{
	};

	// Forward declaration
	template<typename LhsType, typename RhsType = LhsType, typename ReturnType = void, typename Args = WhrapperTypes<>>
	class Dispatcher;

	/**
	 * @brief Dispatcher mapping pairs of dynamic types to callable functions.
	 *
	 * @tparam LhsType Base type for the left‑hand side argument.
	 * @tparam RhsType Base type for the right‑hand side argument.
	 * @tparam ReturnType Return type of the dispatched function.
	 * @tparam Args Variadic list of additional argument types.
	 */
	template<typename LhsType, typename RhsType, typename ReturnType, typename... Args>
	class Dispatcher<LhsType, RhsType, ReturnType, WhrapperTypes<Args...>>
	{
	public:
		/**
		 * @brief Key type used in the internal map.
		 *
		 * The key is a pair of type names obtained from `typeid().name()`.
		 */
		using key_type = std::pair<std::string, std::string>;

		/**
		 * @brief Function type stored in the dispatcher.
		 *
		 * A callable taking `(LhsType&, RhsType&, Args...)` and returning `ReturnType`.
		 */
		using value_type = std::function<ReturnType(LhsType&, RhsType&, Args&&...)>;

		/**
		 * @brief Registers a function for a pair of derived types.
		 *
		 * @tparam DerivedLhsType Dynamic type expected for the left argument.
		 * @tparam DerivedRhsType Dynamic type expected for the right argument.
		 *
		 * @param value Function to associate with the type pair.
		 *
		 * @throws std::runtime_error If a function is already registered for this pair.
		 */
		template<typename DerivedLhsType, typename DerivedRhsType>
		void Add(const value_type& value)
		{
			key_type key = key_type(typeid(DerivedLhsType).name(), typeid(DerivedRhsType).name());
			auto it = m_map.find(key);
			if (it != m_map.end())
				throw std::runtime_error("Already Register");
			m_map[key] = value;
		}

		/**
		 * @brief Dispatches a call based on the dynamic types of the arguments.
		 *
		 * @param lhs Left‑hand side object.
		 * @param rhs Right‑hand side object.
		 * @param args Additional forwarded arguments.
		 *
		 * @return ReturnType Result of the dispatched function.
		 *
		 * @throws std::runtime_error If no function is registered for the dynamic type pair.
		 */
		ReturnType operator()(LhsType& lhs, RhsType& rhs, Args&&... args)
		{
			key_type key = key_type(typeid(lhs).name(), typeid(rhs).name());
			auto it = m_map.find(key);
			if (it == m_map.end())
				throw std::runtime_error("not Register");
			return it->second(lhs, rhs, std::forward<Args>(args)...);
		}

	private:
		std::map<key_type, value_type> m_map;
	};

	// Forward declaration
	template<typename LhsType, typename RhsType = LhsType, typename ReturnType = void, typename Args = WhrapperTypes<>>
	class FNDispatcher;

	/**
	 * @brief High‑level dispatcher that wraps a Dispatcher and registers raw function pointers.
	 *
	 * This class simplifies registration by automatically casting base references
	 * to derived references before calling the user‑provided function.
	 *
	 * @tparam LhsType Base type for the left argument.
	 * @tparam RhsType Base type for the right argument.
	 * @tparam ReturnType Return type of the dispatched function.
	 * @tparam Args Additional argument types.
	 */
	template<typename LhsType, typename RhsType, typename ReturnType, typename... Args>
	class FNDispatcher<LhsType, RhsType, ReturnType, WhrapperTypes<Args...>>
	{
	public:
		/// @brief Alias for the argument type list.
		using CtorList = WhrapperTypes<Args...>;

		/**
		 * @brief Registers a function pointer for a pair of derived types.
		 *
		 * @tparam DerivedLhsType Expected dynamic type for the left argument.
		 * @tparam DerivedRhsType Expected dynamic type for the right argument.
		 * @tparam FN Function pointer to register.
		 * @tparam Mirror If true, also registers the mirrored pair (RhsType, LhsType).
		 *
		 * The function pointer must have the signature:
		 * @code
		 * ReturnType FN(DerivedLhsType&, DerivedRhsType&, Args...);
		 * @endcode
		 *
		 * @throws std::runtime_error If a function is already registered for the type pair.
		 */
		template<typename DerivedLhsType, typename DerivedRhsType, ReturnType(*FN)(DerivedLhsType&, DerivedRhsType&, Args...), bool Mirror = false>
		void Add()
		{
			// Due to inheritance and virtual Destructor cast types to register Derived
			auto fn = [&](LhsType& lhs, RhsType& rhs, Args&&... args)->ReturnType
				{
					return FN(static_cast<DerivedLhsType&>(lhs), static_cast<DerivedRhsType&>(rhs), std::forward<Args>(args)...);
				};
			// Add to dispatcher
			m_dispatcher.template Add<DerivedLhsType, DerivedRhsType>(fn);
			// Due to Compile time bool if constexpr for optimisation
			if constexpr (!Mirror)
				return;
			auto fn2 = [&](RhsType& rhs, LhsType& lhs, Args&&... args) ->ReturnType
				{
					return fn(lhs, rhs, std::forward<Args>(args)...);
				};
			// Add to dispatcher
			m_dispatcher.template Add<DerivedRhsType, DerivedLhsType>(fn2);
		}

		/**
		 * @brief Dispatches a call using the underlying Dispatcher.
		 *
		 * @param lhs Left‑hand side object.
		 * @param rhs Right‑hand side object.
		 * @param args Additional arguments.
		 *
		 * @return ReturnType Result of the dispatched function.
		 */
		ReturnType operator()(LhsType& lhs, RhsType& rhs, Args... args)
		{
			return m_dispatcher(lhs, rhs, std::forward<Args>(args)...);
		}

	private:
		Dispatcher<LhsType, RhsType, ReturnType, CtorList> m_dispatcher;
	};
}