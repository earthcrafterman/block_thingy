#pragma once
#include "Base.hpp"

#include <glm/vec2.hpp>

#include "fwd/game.hpp"

namespace block_thingy::graphics::gui {

class Play : public Base
{
public:
	Play(game&);

	std::string type() const override;

	void init() override;
	void close() override;
	void draw() override;

	void keypress(const util::key_press&) override;
	void mousepress(const util::mouse_press&) override;
	void mousemove(const glm::dvec2& position) override;
	void joypress(int joystick, int button, bool pressed) override;
	void joymove(const glm::dvec2& offset) override;

private:
	void draw_gui() override;
	void draw_crosshair();
	void draw_debug_text();
};

}
