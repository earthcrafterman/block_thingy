#include "GUI.hpp"

#include <glad/glad.h>

#include <glm/gtx/transform.hpp> // glm::ortho
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include "../Gfx.hpp"

GUI::GUI()
	:
	s_crosshair("shaders/crosshair")
{
}

void GUI::update_framebuffer_size(uint_fast32_t width, uint_fast32_t height)
{
	float midX = width / 2.0f;
	float midY = height / 2.0f;
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
	crosshair_vbo.data(sizeof(crosshair_vertex), crosshair_vertex, GL_DYNAMIC_DRAW);
}

void GUI::draw(const Gfx& gfx)
{
	draw_crosshair(gfx);
}

void GUI::draw_crosshair(const Gfx& gfx)
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(s_crosshair.get_name());
	s_crosshair.uniformMatrix4fv("matriks", gfx.matriks_ptr);

	glm::mat4 crosshair_matrix = glm::ortho(0.0f, static_cast<float>(gfx.width), static_cast<float>(gfx.height), 0.0f, -1.0f, 1.0f);
	s_crosshair.uniformMatrix4fv("matriks", glm::value_ptr(crosshair_matrix));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo.get_name());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glDisableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
}
