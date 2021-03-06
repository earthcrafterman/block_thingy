#pragma once
#include "Base.hpp"

#include <cstddef>
#include <string>

#include "fwd/game.hpp"
#include "fwd/block/test_light.hpp"
#include "fwd/graphics/GUI/Widget/TextInput.hpp"
#include "position/block_in_world.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy::graphics::gui {

class Light : public Base
{
public:
	Light(game&, world::world&, block::test_light&, const position::block_in_world&);

	std::string type() const override;

	void init() override;
	void draw() override;

private:
	world::world& world;
	block::test_light& block;
	position::block_in_world block_pos;

	void on_change(std::ptrdiff_t, widget::TextInput&, const std::string&);
};

}
