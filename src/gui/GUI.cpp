#include "GUI.hpp"

#include <glad/glad.h>
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho

#include "Game.hpp"
#include "Gfx.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"

GUI::GUI(EventManager& event_manager)
	:
	s_crosshair("shaders/crosshair")
{
	event_manager.add_handler(EventType::window_size_change, [self=this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		self->update_framebuffer_size(e.window_size);
	});
}

void GUI::update_framebuffer_size(const window_size_t& window_size)
{
	float midX = window_size.x / 2.0f;
	float midY = window_size.y / 2.0f;
	GLfloat crosshair_vertex[] = {
		midX - 16, midY - 1, 0,
		midX - 16, midY + 1, 0,
		midX + 16, midY + 1, 0,
		midX - 16, midY - 1, 0,
		midX + 16, midY + 1, 0,
		midX + 16, midY - 1, 0,

		midX - 1, midY - 16, 0,
		midX - 1, midY + 16, 0,
		midX + 1, midY + 16, 0,
		midX - 1, midY - 16, 0,
		midX + 1, midY + 16, 0,
		midX + 1, midY - 16, 0,
	};
	const auto usage_hint = Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw;
	crosshair_vbo.data(sizeof(crosshair_vertex), crosshair_vertex, usage_hint);
}

void GUI::draw(Gfx& gfx)
{
	draw_crosshair(gfx);
}

void GUI::draw_crosshair(const Gfx& gfx)
{
	bool wireframe = Game::instance->wireframe();

	if(wireframe)
	{
		Game::instance->wireframe = false;
	}

	glDisable(GL_DEPTH_TEST);

	glUseProgram(s_crosshair.get_name());

	const float width = gfx.window_size.x;
	const float height = gfx.window_size.y;
	glm::mat4 crosshair_matrix = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	s_crosshair.uniform("matriks", crosshair_matrix);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo.get_name());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	if(wireframe)
	{
		Game::instance->wireframe = true;
	}
}
