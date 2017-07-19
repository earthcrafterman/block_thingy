#pragma once

#include <cassert>
#include <map>
#include <stdint.h>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "fwd/block/Enum/Type.hpp"
#include "fwd/event/EventManager.hpp"
#include "graphics/RenderTarget.hpp"
#include "graphics/Text.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"
#include "fwd/physics/AABB.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "types/window_size_t.hpp"

class Gfx
{
public:
	Gfx(GLFWwindow*);

	Gfx(Gfx&&) = delete;
	Gfx(const Gfx&) = delete;
	void operator=(const Gfx&) = delete;

	static Gfx* instance;
private:
	struct set_instance
	{
		set_instance(Gfx* ptr)
		{
			assert(Gfx::instance == nullptr);
			Gfx::instance = ptr;
		}
	} set_instance;

public:
	GLFWwindow* window;
	window_size_t window_size;
	glm::dvec2 window_mid;

	// I should use less global state here
	glm::dmat4 projection_matrix;
	glm::dmat4 view_matrix_physical;
	glm::dmat4 view_matrix_graphical;
	glm::dvec3 physical_position;
	glm::dvec3 graphical_position;

	glm::dmat4 vp_matrix; // view (graphical) and projection

	std::map<Block::Enum::Type, Graphics::OpenGL::ShaderProgram> block_shaders;

	Graphics::OpenGL::ShaderProgram s_lines;

	Graphics::OpenGL::VertexBuffer outline_vbo;
	Graphics::OpenGL::VertexArray outline_vao;

	Graphics::Text gui_text;
	glm::dmat4 gui_projection_matrix;

	void hook_events(EventManager&);
	static GLFWwindow* init_glfw();
	static void uninit_glfw(GLFWwindow*);
	void opengl_setup();

	void update_framebuffer_size(const window_size_t&);

	Graphics::RenderTarget screen_rt;
	Graphics::RenderTarget buf_rt;
	std::map<std::string, Graphics::OpenGL::ShaderProgram> screen_shaders;
	Graphics::OpenGL::ShaderProgram* screen_shader;
	Graphics::OpenGL::VertexBuffer quad_vbo;
	Graphics::OpenGL::VertexArray quad_vao;
	void set_screen_shader(const std::string&);

	/**
	 * Set borderless window (true) or normal window (false)
	 *
	 * @todo Add real fullscreen mode
	 */
	void set_fullscreen(bool);

	void set_cull_face(bool);
	void update_projection_matrix();
	static glm::dmat4 make_projection_matrix(double width, double height);
	static glm::dmat4 make_rotation_matrix(const glm::dvec3& rotation);
	void set_camera_view
	(
		const glm::dvec3& position,
		const glm::dmat4& rotation,
		const glm::dmat4& projection_matrix
	);
	void draw_box_outline(const glm::dvec3& min, const glm::dvec3& max, const glm::dvec4& color);
	void draw_box_outline(const Physics::AABB&, const glm::dvec4& color);
	void draw_block_outline(const Position::BlockInWorld&, const glm::dvec4& color);
	Graphics::OpenGL::ShaderProgram& get_block_shader(Block::Enum::Type);

	void center_cursor();

	// for GUIs
	Graphics::OpenGL::ShaderProgram s_gui_shape;
	Graphics::OpenGL::VertexBuffer gui_rectangle_vbo;
	Graphics::OpenGL::VertexArray gui_rectangle_vao;
	void draw_rectangle(glm::dvec2 position, glm::dvec2 size, const glm::dvec4& color);
	void draw_border(glm::dvec2 position, glm::dvec2 size, glm::dvec4 border_size, const glm::dvec4& color);
};
