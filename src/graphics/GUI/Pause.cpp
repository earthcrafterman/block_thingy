#include "Pause.hpp"

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"

using std::string;

namespace Graphics::GUI {

Pause::Pause(Game& game)
:
	Base(game, "guis/Pause.btgui")
{
}

string Pause::type() const
{
	return "Pause";
}

void Pause::init()
{
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Pause::draw()
{
	parent->draw();

	GLboolean depth_test;
	glGetBooleanv(GL_DEPTH_TEST, &depth_test);
	if(depth_test) glDisable(GL_DEPTH_TEST);

	Gfx::instance->draw_rectangle({0, 0}, static_cast<glm::dvec2>(Gfx::instance->window_size), {0, 0, 0, 0.3});

	if(depth_test) glEnable(GL_DEPTH_TEST);

	Base::draw();
}

}
