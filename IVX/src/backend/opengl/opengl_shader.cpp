#include "ivxpch.h"
#include "opengl_shader.h"

#include <glad/glad.h>

namespace ivx {
OpenGLShader::OpenGLShader(const std::string &vs_src,
                           const std::string &fs_src) {
    //// Read our shaders into the appropriate buffers
    // std::string vertexSource = // Get source code for vertex shader.
    //	std::string fragmentSource = // Get source code for fragment shader.

    // Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Send the vertex shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    const GLchar *source = reinterpret_cast<const GLchar *>(vs_src.c_str());
    glShaderSource(vertexShader, 1, &source, 0);

    // Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

        // We don't need the shader anymore.
        glDeleteShader(vertexShader);

        // Use the infoLog as you see fit.
        IVX_CORE_ERROR("{}", infoLog.data());
        IVX_CORE_ASSERT(false, "Vertex hader link failure!");
        // In this simple program, we'll just leave
        return;
    }

    // Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Send the fragment shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    source = reinterpret_cast<const GLchar *>(fs_src.c_str());
    glShaderSource(fragmentShader, 1, &source, 0);

    // Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

        // We don't need the shader anymore.
        glDeleteShader(fragmentShader);
        // Either of them. Don't leak shaders.
        glDeleteShader(vertexShader);

        // Use the infoLog as you see fit.
        IVX_CORE_ERROR("{}", infoLog.data());
        IVX_CORE_ASSERT(false, "Fragment hader link failure!");
        // In this simple program, we'll just leave
        return;
    }

    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    GLuint program = glCreateProgram();
    program_ = program;

    // Attach our shaders to our program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Link our program
    glLinkProgram(program);

    // Note the different functions here: glGetProgram* instead of
    // glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        // We don't need the program anymore.
        glDeleteProgram(program);
        // Don't leak shaders either.
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Use the infoLog as you see fit.

        // In this simple program, we'll just leave
        return;
    }

    // Always detach shaders after a successful link.
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    glUseProgram(0);
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(program_);
}

void OpenGLShader::bind() {
    glUseProgram(program_);
}

void OpenGLShader::unbind() {
    glUseProgram(0);
}

int OpenGLShader::location(const std::string &name) const {
    return glGetUniformLocation(program_, name.c_str());
}

void OpenGLShader::set(const std::string &name, bool val) const {
    glUseProgram(program_);
    glUniform1i(location(name), static_cast<GLint>(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, float val) const {
    glUseProgram(program_);
    glUniform1f(location(name), val);
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::vec2 &val) const {
    glUseProgram(program_);
    glUniform2fv(location(name), 1, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::vec3 &val) const {
    glUseProgram(program_);
    glUniform3fv(location(name), 1, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::vec4 &val) const {
    glUseProgram(program_);
    glUniform4fv(location(name), 1, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::ivec2 &val) const {
    glUseProgram(program_);
    glUniform2iv(location(name), 1, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::ivec3 &val) const {
    glUseProgram(program_);
    glUniform3iv(location(name), 1, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::ivec4 &val) const {
    glUseProgram(program_);
    glUniform4iv(location(name), 1, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::mat3 &val) const {
    glUseProgram(program_);
    glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(val));
    glUseProgram(0);
}

void OpenGLShader::set(const std::string &name, const glm::mat4 &val) const {
    glUseProgram(program_);
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(val));
    glUseProgram(0);
}
} // namespace ivx