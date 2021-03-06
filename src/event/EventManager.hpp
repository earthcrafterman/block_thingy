#pragma once

#include <functional>
#include <mutex>
#include <stdint.h>
#include <unordered_map>
#include <utility>

#include "fwd/event/Event.hpp"
#include "fwd/event/EventType.hpp"

namespace block_thingy {

using event_handler_t = std::function<void(const Event&)>;
using event_handler_id_t = uint_fast32_t;

// EventDispatcher?
class EventManager
{
public:
	EventManager();

	EventManager(EventManager&&) = delete;
	EventManager(const EventManager&) = delete;
	EventManager& operator=(EventManager&&) = delete;
	EventManager& operator=(const EventManager&) = delete;

	event_handler_id_t add_handler(const event_handler_t&);
	event_handler_id_t add_handler(EventType, const event_handler_t&);
	void unadd_handler(event_handler_id_t);
	void do_event(const Event&) const;

private:
	event_handler_id_t max_id;
	std::unordered_map<event_handler_id_t, std::pair<EventType, event_handler_t>> handlers;
	mutable std::mutex mutex;
};

}
