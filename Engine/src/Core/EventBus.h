#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include "Core/Event.h"
#include "Utility/Memory.h"

namespace engi
{

	class EventDispatcher
	{
	public:
		EventDispatcher() = default;
		virtual ~EventDispatcher() = default;
		
		void dispatch(const Event& e) const
		{
			invokeHandler(e);
		}
	private:
		virtual void invokeHandler(const Event& e) const = 0;
	};

	template<typename Receiver, typename EventType>
	class EventHandler : public EventDispatcher
	{
	public:
		using HandlerFn = void(Receiver::*)(const EventType&);

		EventHandler(Receiver* receiver, HandlerFn handlerFn)
			: m_Receiver(receiver), m_HandlerFn(handlerFn) {}

		virtual void invokeHandler(const Event& e) const override
		{
			(m_Receiver->*m_HandlerFn)(static_cast<const EventType&>(e));
		}

		bool operator==(const EventHandler<Receiver, EventType>& rhs) const
		{
			return (m_Receiver == rhs.m_Receiver)
				&& (m_HandlerFn == rhs.m_HandlerFn);
		}
		bool operator==(Receiver* rhs) const
		{
			return m_Receiver == rhs;
		}
	private:
		Receiver* m_Receiver;
		HandlerFn m_HandlerFn;
	};

	class EventBus
	{
	public:
		using Dispatcher = UniqueHandle<EventDispatcher>;
		using HandlerList = std::vector<Dispatcher>;
		template<typename Receiver, typename EventType>
		using HandlerFn = void(Receiver::*)(const EventType&);

		EventBus() = default;
		EventBus(const EventBus&) = delete;
		EventBus& operator=(const EventBus&) = delete;
		~EventBus() = default;

		static inline EventBus& get() noexcept { return *s_eventBus; }
		
		static bool init() noexcept;
		static void deinit() noexcept;

		template<typename EventType>
		void publish(const EventType& e) noexcept
		{
			auto entry = m_Subscriptions.find(typeid(EventType));
			if (entry == m_Subscriptions.end())
				return;

			for (const auto& handler : entry->second)
			{
				if (e.handled) 
					return;
				handler->dispatch(e);
			}
		}

		template<typename Receiver, typename EventType>
		void subscribe(Receiver* receiver, void(Receiver::* handlerFn)(const EventType&))
		{
			HandlerList& handlers = m_Subscriptions[typeid(EventType)];
			handlers.emplace_back(makeUnique<EventHandler<Receiver, EventType>>(new EventHandler<Receiver, EventType>(receiver, handlerFn)));
		}

		template<typename Receiver, typename EventType>
		void unsubscribe(Receiver* receiver, void(Receiver::* handlerFn)(const EventType&))
		{
			auto entry = m_Subscriptions.find(typeid(EventType));
			if (entry != m_Subscriptions.end())
			{
				const EventHandler<Receiver, EventType> tmp = { receiver, handlerFn };
				HandlerList& handlers = entry->second;
				std::erase_if(handlers, [&](const auto& handler)
					{
						using T = EventHandler<Receiver, EventType>;
						return *static_cast<T*>(handler.get()) == tmp;
					});
			}
		}

		template<typename Receiver>
		void unsubscribeAll(Receiver* receiver)
		{
			for (auto& [_, handlers] : m_Subscriptions)
			{
				std::erase_if(handlers, [&](const auto& handler)
					{
						using T = EventHandler<Receiver, Event>;
						return *static_cast<T*>(handler.get()) == receiver;
					});
			}
		}
	private:
		std::unordered_map<std::type_index, HandlerList> m_Subscriptions;
		inline static EventBus* s_eventBus = nullptr;
	};

}; // engi namespace