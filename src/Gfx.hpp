#pragma once

#include <cstdint>

#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include "Coords.hpp"

class Camera;
struct GLFWwindow;

class Gfx
{
	public:
		explicit Gfx(GLFWwindow* window);

		GLFWwindow* window;
		uint_fast32_t width;
		uint_fast32_t height;
		glm::mat4 projection_matrix;
		glm::mat4 view_matrix;

		GLuint vertex_array;
		glm::mat4 matriks;
		GLfloat* matriks_ptr;

		GLuint sp_cube;
		GLint vs_cube_matriks;
		GLint vs_cube_pos_mod;

		GLuint sp_lines;
		GLint vs_lines_matriks;
		GLint vs_lines_color;

		GLuint sp_crosshair;
		GLint vs_crosshair_matriks;

		GLuint outline_vbo;

		static void init_glfw();
		void opengl_setup();
		void opengl_cleanup();
		void update_framebuffer_size();
		void set_cam_view(const Camera& cam);

		void draw_cube_outline(Position::BlockInWorld pos, const glm::vec4& color = glm::vec4(1, 1, 1, 1));

		static void write_png_RGB(const char* filename, uint8_t* buf, uint32_t width, uint32_t height, bool reverse_rows = false);

	private:
		void update_projection_matrix();
};