#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

class Event;
enum class EventType;

using event_handler_t = std::function<void(const Event&)>;
using event_handler_id_t = uint_fast32_t;

// EventDispatcher?
class EventManager
{
	public:
		EventManager();

		event_handler_id_t add_handler(event_handler_t);
		event_handler_id_t add_handler(EventType, event_handler_t);
		void unadd_handler(event_handler_id_t);
		void do_event(const Event&) const;

	private:
		event_handler_id_t max_id;
		std::unordered_map<event_handler_id_t, std::pair<EventType, event_handler_t>> handlers;
};