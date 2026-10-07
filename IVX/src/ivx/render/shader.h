#pragma once

#include <glad/glad.h>
#include <string_view>

namespace ivx {
	class Shader {
	public:
		Shader(const std::string& vs_src, const std::string& fs_src);
		~Shader();

		void bind();
		void unbind();

	private:
		GLuint program_{ 0 };
	};
}