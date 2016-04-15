#pragma once

#include <string>

#include <glad/glad.h>

class ShaderObject
{
	public:
		ShaderObject();
		ShaderObject(const std::string& file_path, GLenum type);
		~ShaderObject();

		ShaderObject(ShaderObject&&);
		ShaderObject(const ShaderObject&) = delete;
		void operator=(const ShaderObject&) = delete;

		GLuint get_name() const;

	private:
		bool inited;
		GLuint name;
};
