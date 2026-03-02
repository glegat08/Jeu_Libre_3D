#pragma once
#include <deque>
#include <memory>
#include <map>
#include <iostream>
#include "EventDispatcher.h"

/**
 * @brief Central event bus responsible for queuing and dispatching events.
 *
 * Responsibilities:
 * - Stores all registered receivers (objects able to process events).
 * - Stores emitted events in a FIFO queue.
 * - Dispatches each event to receivers in priority order using EventDispatcher.
 * - Stops propagation when `evt.m_handled == true`.
 *
 * This is the core of the event system: receivers register themselves,
 * events are emitted asynchronously, and Dispatch() processes them.
 */
struct EventBus
{
public:
    /// @brief Alias for the dispatcher used internally.
    using Dispatcher = EventDispatcher;

    /**
     * @brief Registers a receiver with an optional priority.
     *
     * Receivers with lower priority values are processed first.
     *
     * @param receiver Pointer to the receiver object.
     * @param priority Priority value (lower = earlier dispatch).
     */
    void Register(Receiver* receiver, int priority = 0)
    {
        m_receivers.emplace(priority, receiver);
    }

    /**
     * @brief Unregisters a receiver from the bus.
     *
     * @param receiver Pointer to the receiver to remove.
     */
    void Unregister(Receiver* receiver)
    {
        for (auto it = m_receivers.begin(); it != m_receivers.end();)
        {
            if (it->second == receiver)
                it = m_receivers.erase(it);
            else
                ++it;
        }
    }

    /**
     * @brief Emits an event by constructing it and pushing it into the queue.
     *
     * The event is dynamically allocated and stored until Dispatch() is called.
     *
     * @tparam EventType Type of the event to create.
     * @tparam Args Constructor argument types.
     *
     * @param args Arguments forwarded to the event constructor.
     */
    template<typename EventType, typename ...Args>
    void Emit(Args&& ... args)
    {
        m_queue.emplace_back(std::make_unique<EventType>(std::forward<Args>(args)...));
    }

    /**
     * @brief Processes all pending events in the queue.
     *
     * For each event:
     * - It is popped from the queue.
     * - It is dispatched to receivers in priority order.
     * - If a receiver handles the event (`m_handled == true`), propagation stops.
     *
     * Exceptions thrown by receivers or the dispatcher are caught and ignored.
     */
    void Dispatch()
    {
        while (!m_queue.empty())
        {
            auto evt = std::move(m_queue.front());
            m_queue.pop_front();

            for (auto& [priority, receiver] : m_receivers)
            {
                try
                {
                    m_dispatcher(*receiver, *evt);
                }
                catch (...)
                {
					std::cerr << "Error dispatching event to receiver at priority " << priority << std::endl;
                }

                if (evt->m_handled)
                    break;
            }
        }
    }

    /**
     * @brief Accessor for the internal dispatcher.
     *
     * Allows receivers to register handlers for specific event types.
     *
     * @return Reference to the internal EventDispatcher.
     */
    Dispatcher& GetDispatcher()
    {
        return m_dispatcher;
    }

private:
    std::deque<std::unique_ptr<Event>> m_queue; ///< FIFO queue of pending events.

    /**
     * @brief Registered receivers sorted by priority.
     *
     * Lower priority values are processed first.
     */
    std::multimap<int, Receiver*> m_receivers;

    Dispatcher m_dispatcher; ///< Internal dispatcher used to route events.
};