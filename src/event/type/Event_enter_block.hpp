#pragma once
#include "event/Event.hpp"

#include <memory>

#include "fwd/Player.hpp"
#include "fwd/block/base.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy {

class Event_enter_block : public Event
{
public:
	Event_enter_block
	(
		world::world&,
		Player&,
		std::shared_ptr<block::base>
	);

	world::world& world;
	Player& player;
	std::shared_ptr<block::base> block;
};

}
