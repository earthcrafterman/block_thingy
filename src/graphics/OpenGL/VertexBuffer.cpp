#include "VertexBuffer.hpp"

#include <utility>

#include <glad/glad.h>

namespace Graphics::OpenGL {

VertexBuffer::VertexBuffer(Format format)
:
	VertexBuffer(std::vector<Format>{std::move(format)})
{
}

VertexBuffer::VertexBuffer(std::vector<Format> formats)
:
	formats(std::move(formats))
{
	glCreateBuffers(1, &name);
	inited = true;
}

VertexBuffer::VertexBuffer(VertexBuffer&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
		formats = std::move(that.formats);
	}
}

VertexBuffer::~VertexBuffer()
{
	if(inited)
	{
		glDeleteBuffers(1, &name);
	}
}

GLuint VertexBuffer::get_name()
{
	return name;
}

void VertexBuffer::data(const std::size_t size, const void* data, const UsageHint usage)
{
	glNamedBufferData(name, size, data, static_cast<GLenum>(usage));
}

} // namespace Graphics::OpenGL
