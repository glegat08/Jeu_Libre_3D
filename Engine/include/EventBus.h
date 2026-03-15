#pragma once
#include <vector>
#include <stdexcept>
#include <memory>
#include <RTTI.h>

namespace KGR
{
	/**
	 * @brief Event bus responsible for dispatching events of a specific type.
	 *
	 * @tparam EventType Type of event handled by this bus.
	 */
	template<typename EventType>
	class EventBus;

	/**
	 * @brief Internal interface representing a container of listeners for a given event type.
	 *
	 * This abstract structure is used by the EventBus to store
	 * different types of listeners able to react to a specific event.
	 *
	 * @tparam EventType Type of the event.
	 */
	template<typename EventType>
	struct IEventHolder
	{
		/// Allows EventBus to access private and protected members
		friend EventBus<EventType>;

		/**
		 * @brief Virtual destructor.
		 */
		virtual ~IEventHolder();

	protected:

		/**
		 * @brief Notifies listeners of an event.
		 *
		 * Must be implemented by derived classes.
		 *
		 * @param event Event to dispatch.
		 */
		virtual void Notify(const EventType& event) = 0;

		/**
		 * @brief Protected constructor.
		 *
		 * @param id Internal identifier of the holder.
		 */
		IEventHolder(int id);

	private:

		/**
		 * @brief Returns the holder identifier.
		 *
		 * @return Internal ID.
		 */
		int GetID() const;

		/// Holder identifier
		int m_id;
	};


	/**
	 * @brief Concrete implementation of an event holder for a specific listener type.
	 *
	 * This class stores listeners and their associated callbacks
	 * in order to notify them when an event occurs.
	 *
	 * @tparam Type Listener type.
	 * @tparam EventType Event type.
	 */
	template<typename Type, typename EventType>
	struct EventHolder : public IEventHolder<EventType>
	{
		/// Allows EventBus to access private members
		friend EventBus<EventType>;

		/**
		 * @brief Default constructor.
		 */
		EventHolder();

		/**
		 * @brief Member function callback type.
		 */
		using CallBack = void(Type::*)(const EventType& event);

	private:

		/**
		 * @brief Adds a callback to the callback list.
		 *
		 * @param cb Member function called when an event is dispatched.
		 */
		void AddCallBack(CallBack cb);

		/**
		 * @brief Registers a listener.
		 *
		 * @param listener Listener instance.
		 */
		void AddListener(Type* listener);

		/**
		 * @brief Removes a listener.
		 *
		 * @param listener Listener instance to remove.
		 */
		void RemoveListener(Type* listener);

		/**
		 * @brief Notifies all registered listeners.
		 *
		 * @param event Event to dispatch.
		 */
		void Notify(const EventType& event) override final;

		/// List of listener instances
		std::vector<Type*> m_listeners;

		/// List of callbacks associated with listeners
		std::vector<CallBack> m_callBack;
	};

	/**
	 * @brief Static event bus used to manage listeners and event notifications.
	 *
	 * This class allows registering objects listening to a specific event type
	 * and notifying them when an event is triggered.
	 *
	 * @tparam EventType Type of event handled by the bus.
	 */
	template<typename EventType>
	class EventBus
	{
	public:

		/**
		 * @brief Alias for a listener callback.
		 *
		 * @tparam Type Listener type.
		 */
		template<typename Type>
		using CallBack = void(Type::*)(const EventType& event);

		/**
		 * @brief Adds a listener to the event bus.
		 *
		 * @tparam Type Listener type.
		 * @param listener Listener instance to register.
		 */
		template<typename Type>
		static void AddListener(Type* listener);

		/**
		 * @brief Removes a listener from the event bus.
		 *
		 * @tparam Type Listener type.
		 * @param listener Listener instance to remove.
		 */
		template<typename Type>
		static void RemoveListener(Type* listener);

		/**
		 * @brief Registers a callback for a specific listener type.
		 *
		 * @tparam Type Listener type.
		 * @param cb Callback called when an event occurs.
		 */
		template<typename Type>
		static void AddCallBack(CallBack<Type> cb);

		/**
		 * @brief Notifies all registered listeners of an event.
		 *
		 * @param event Event to dispatch.
		 */
		static void Notify(const EventType& event);

		/**
		* @brief Constructs an event in place and notifies all registered listeners.
		*
		* This function forwards the provided arguments to the constructor of
		* the event type in order to create the event instance. The constructed
		* event is then dispatched to every listener registered in the EventBus.
		*
		* @tparam Args Types of the arguments forwarded to the event constructor.
		* @param args Arguments used to construct the event.
		*/
		template<typename... Args>
		static void EmplaceNotify(Args&&... args);

	private:

		/// List of holders containing the different listener types
		static std::vector<std::unique_ptr<IEventHolder<EventType>>> m_listeners;
	};

	template<typename EventType>
	std::vector<std::unique_ptr<IEventHolder<EventType>>> EventBus<EventType>::m_listeners;

	template <typename EventType>
	IEventHolder<EventType>::~IEventHolder() = default;

	template <typename EventType>
	IEventHolder<EventType>::IEventHolder(int id) :m_id(id)
	{
	}

	template <typename EventType>
	int IEventHolder<EventType>::GetID() const
	{
		return m_id;
	}

	template <typename Type, typename EventType>
	EventHolder<Type, EventType>::EventHolder() : IEventHolder<EventType>(RTTI::Counter::GetTypeId<Type>())
	{
	}

	template <typename Type, typename EventType>
	void EventHolder<Type, EventType>::AddCallBack(CallBack cb)
	{
		auto it = std::find(m_callBack.begin(), m_callBack.end(), cb);
		if (it == m_callBack.end())
			m_callBack.push_back(cb);
	}

	template <typename Type, typename EventType>
	void EventHolder<Type, EventType>::AddListener(Type* listener)
	{
		auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
		if (it != m_listeners.end())
			throw std::runtime_error("Listener already added");
		m_listeners.push_back(listener);
	}

	template <typename Type, typename EventType>
	void EventHolder<Type, EventType>::RemoveListener(Type* listener)
	{
		auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
		if (it == m_listeners.end())
			throw std::runtime_error("Listener not found");
		m_listeners.erase(it);
	}

	template <typename Type, typename EventType>
	void EventHolder<Type, EventType>::Notify(const EventType& event)
	{
		for (auto* listner : m_listeners)
		{
			if (!listner)
				continue;
			for (auto& cb : m_callBack)
			{
				if (cb)
					(listner->*cb)(event);
			}
		}
	}

	template <typename EventType>
	template <typename Type>
	void EventBus<EventType>::AddListener(Type* listener)
	{
		auto it = std::find_if(m_listeners.begin(), m_listeners.end(),
			[](const std::unique_ptr<IEventHolder<EventType>>& l) { return l->GetID() == RTTI::Counter::GetTypeId<Type>(); });

		if (it == m_listeners.end())
		{
			auto linker = std::make_unique<EventHolder<Type, EventType>>();
			linker->AddListener(listener);
			m_listeners.push_back(std::move(linker));
		}
		else
		{
			EventHolder<Type, EventType>* linker = static_cast<EventHolder<Type, EventType>*>((*it).get());
			linker->AddListener(listener);
		}
	}

	template <typename EventType>
	template <typename Type>
	void EventBus<EventType>::RemoveListener(Type* listener)
	{
		auto it = std::find_if(m_listeners.begin(), m_listeners.end(),
			[&](const std::unique_ptr<IEventHolder<EventType>>& l) { return l->GetID() == RTTI::Counter::GetTypeId<Type>(); });
		if (it != m_listeners.end())
		{
			EventHolder<Type, EventType>* linker = static_cast<EventHolder<Type, EventType>*>((*it).get());
			linker->RemoveListener(listener);
		}
		else
		{
			throw std::runtime_error("EventHolder not found");
		}
	}

	template <typename EventType>
	template <typename Type>
	void EventBus<EventType>::AddCallBack(CallBack<Type> cb)
	{
		auto it = std::find_if(m_listeners.begin(), m_listeners.end(),
			[&](const std::unique_ptr<IEventHolder<EventType>>& l) { return l->GetID() == RTTI::Counter::GetTypeId<Type>(); });
		if (it != m_listeners.end())
		{
			EventHolder<Type, EventType>* linker = static_cast<EventHolder<Type, EventType>*>((*it).get());
			linker->AddCallBack(cb);
		}
		else
		{
			throw std::runtime_error("EventHolder not found");
		}
	}

	template <typename EventType>
	void EventBus<EventType>::Notify(const EventType& event)
	{
		for (auto& listener : m_listeners)
			listener->Notify(event);
	}

	template <typename EventType>
	template <typename ... Args>
	void EventBus<EventType>::EmplaceNotify(Args&&... args)
	{
		auto event = EventType(std::forward<Args>(args)...);
		for (auto& listener : m_listeners)
			listener->Notify(event);
	}
}