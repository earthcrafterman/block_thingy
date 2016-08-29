#include "Pause.hpp"

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Widget/Button.hpp"

namespace Graphics {
namespace GUI {

Pause::Pause(Game& game)
	:
	Base(game, WidgetContainerMode::widgets)
{
	root.add<Widget::Button>("Resume",
	[&game]()
	{
		game.console.run_line("close_gui");
	});

	root.add<Widget::Button>("Save",
	[&game]()
	{
		game.console.run_line("save");
	});

	root.add<Widget::Button>("Save & Quit",
	[&game]()
	{
		game.console.run_line("save");
		game.console.run_line("quit");
	});

	root.add<Widget::Button>("Quit",
	[&game]()
	{
		game.console.run_line("quit");
	});
}

void Pause::init()
{
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Pause::draw()
{
	parent->draw();
	Base::draw();
}

void Pause::draw_gui()
{
	game.gfx.gui_text.draw("paused", {8, 150});
}

} // namespace GUI
} // namespace Graphics
