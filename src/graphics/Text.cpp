#include "Text.hpp"

#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma clang diagnostic pop

#include "graphics/OpenGL/Texture.hpp"

using std::cerr;
using std::string;

// based on:
// http://learnopengl.com/#!In-Practice/Text-Rendering
// http://www.learnopengl.com/#!In-Practice/2D-Game/Render-text

namespace Graphics {

Text::Character load_char(const FT_Face& face, const FT_ULong c);

Text::Text(const string& font_path, const FT_UInt height)
	:
	height(height),
	vbo({4, GL_FLOAT}),
	vao(vbo),
	shader("shaders/text")
{
	FT_Library ft;
	if(FT_Init_FreeType(&ft))
	{
		cerr << "failed to init FreeType\n";
		return;
	}

	FT_Face face;
	if(FT_New_Face(ft, font_path.c_str(), 0, &face))
	{
		cerr << "failed to load font " << font_path << "\n";
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for(FT_ULong c = ' '; c <= '~'; ++c)
	{
		chars.emplace(c, load_char(face, c));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	shader.uniform("color", glm::vec3(1.0));
}

void Text::draw(const string& s, glm::dvec2 pos, const glm::dmat4& projection_matrix)
{
	const double start_x = pos.x;

	glUseProgram(shader.get_name());
	shader.uniform("projection", glm::mat4(projection_matrix));

	glActiveTexture(GL_TEXTURE0);

	uint_fast32_t line_i = 0;
	for(const char c : s)
	{
		if(c == '\n')
		{
			pos.x = start_x;
			pos.y += height;
			line_i = 0;
			continue;
		}
		if(c == '\t')
		{
			// TODO?: elastic tabstops
			uint_fast8_t width = tab_width - (line_i % tab_width);
			pos.x += chars[' '].x_offset * width;
			line_i += width;
			continue;
		}
		++line_i;

		auto ci = chars.find(c);
		if(ci == chars.cend())
		{
			// TODO
			ci = chars.find('?');
		}
		Character& ch = ci->second;

		float xpos = static_cast<float>(pos.x + ch.bearing.x);
		float ypos = static_cast<float>(pos.y + (chars['H'].bearing.y - ch.bearing.y));

		const float w = ch.size.x;
		const float h = ch.size.y;
		const float y1 = ch.flip ? 1.0f : 0.0f;
		const float y2 = 1.0f - ch.flip;
		float vertices[6][4] =
		{
			{ xpos,     ypos + h, 0.0f, y2 },
			{ xpos,     ypos,     0.0f, y1 },
			{ xpos + w, ypos,     1.0f, y1 },

			{ xpos,     ypos + h, 0.0f, y2 },
			{ xpos + w, ypos,     1.0f, y1 },
			{ xpos + w, ypos + h, 1.0f, y2 },
		};

		glBindTexture(GL_TEXTURE_2D, ch.texture.get_name());

		vbo.data(6 * 4 * sizeof(float), vertices, OpenGL::VertexBuffer::UsageHint::dynamic_draw);
		vao.draw(GL_TRIANGLES, 0, 6);

		pos.x += ch.x_offset;
	}
}

Text::Character load_char(const FT_Face& face, const FT_ULong c)
{
	if(FT_Load_Char(face, c, FT_LOAD_RENDER))
	{
		cerr << "failed to load character: " << c << "\n";
		return
		{
			{},
			{0, 0},
			{0, 0},
			0,
			false,
		};
	}

	const auto& bitmap = face->glyph->bitmap;

	return
	{
		{
			GL_TEXTURE_2D,
			bitmap.buffer,
			bitmap.width,
			bitmap.rows,
			GL_UNSIGNED_BYTE,
		},
		glm::ivec2(bitmap.width, bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		face->glyph->advance.x / 64.0,
		bitmap.pitch < 0,
	};
}

} // namespace Graphics
