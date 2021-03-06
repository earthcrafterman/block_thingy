#pragma once

#include <memory>
#include <string>

#include <glm/vec2.hpp>

#include "fwd/game.hpp"
#include "fwd/event/EventManager.hpp"
#include "graphics/GUI/Widget/Container.hpp"
#include "types/window_size_t.hpp"
#include "fwd/util/char_press.hpp"
#include "util/filesystem.hpp"
#include "fwd/util/key_press.hpp"
#include "fwd/util/mouse_press.hpp"

namespace block_thingy::graphics::gui {

class Base
{
public:
	Base
	(
		game&,
		const fs::path& layout_path
	);
	virtual ~Base();

	Base(Base&&) = delete;
	Base(const Base&) = delete;
	Base& operator=(Base&&) = delete;
	Base& operator=(const Base&) = delete;

	virtual std::string type() const = 0;

	virtual void init();
	virtual void close();
	virtual void draw();

	virtual void keypress(const util::key_press&);
	virtual void charpress(const util::char_press&);
	virtual void mousepress(const util::mouse_press&);
	virtual void mousemove(const glm::dvec2& position);
	virtual void joypress(int joystick, int button, bool pressed);
	virtual void joymove(const glm::dvec2& offset);

	std::unique_ptr<Base> parent;

	game& g;

protected:
	virtual void draw_gui();
	virtual void update_framebuffer_size(const window_size_t&);
	widget::Container root;

private:
	event_handler_id_t event_handler;
};

}
