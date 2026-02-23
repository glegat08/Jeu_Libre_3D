#pragma once
#include <deque>
#include <memory>
#include <map>
#include "EventDispatcher.h"

// EventBus :
/**
 * - stock every receivers (object that receive the events)
 * - stock the events in the queue
 * - distrib every events to every receivers with the EventDispatcher
 * - stop if evt.handled == true
 */
struct EventBus
{
public:
	using Dispatcher = EventDispatcher;

	//Add a receiver into the list
	void Register(Receiver* receiver, int priority = 0)
	{
		m_receivers.emplace(priority, receiver);
	}

	//Remove a receiver from the list
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

	//Create an Event and ad it to the queue
	template<typename EventType, typename ...Args>
	void Emit(Args&& ... args)
	{
		//create a dynamic event and add it into the FIFO queue
		m_queue.emplace_back(std::make_unique<EventType>(std::forward<Args>(args)...));
	}

	//Process all events waitings
	void Dispatch()
	{
		//while events in queue
		while (!m_queue.empty())
		{
			//take the first event
			auto evt = std::move(m_queue.front());
			m_queue.pop_front();

			//send to every receivers
			for (auto& [priority, receiver] : m_receivers)
			{
				try
				{
					//dispatch
					m_dispatcher(*receiver, *evt);
				}
				catch (...)
				{
					//no handler so dispatcher catch an execption
				}

				//the event is handled so we break
				if (evt->handled)
					break;
			}
		}
	}

	Dispatcher& GetDispatcher()
	{
		return m_dispatcher;
	}

private:
	std::deque<std::unique_ptr<Event>> m_queue;
	//sorted by priority
	std::multimap<int, Receiver*> m_receivers;
	Dispatcher m_dispatcher;
};