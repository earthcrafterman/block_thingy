#pragma once
#include "Base.hpp"

#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

namespace Graphics {
namespace GUI {

class Play : public Base
{
	public:
		Play(Game&);

		void init() override;
		void close() override;
		void draw() override;

		void keypress(int key, int scancode, int action, int mods) override;
		void mousepress(int button, int action, int mods) override;
		void mousemove(double x, double y) override;

	private:
		void draw_gui() override;
		void draw_crosshair();
		void draw_debug_text();
		void update_framebuffer_size(const window_size_t&) override;

		Graphics::OpenGL::ShaderProgram s_crosshair;
		Graphics::OpenGL::VertexBuffer crosshair_vbo;
		Graphics::OpenGL::VertexArray crosshair_vao;
};

} // namespace GUI
} // namespace Graphics
