#include "ivxpch.h"
#include "opengl_shader.h"
#include "ivx/core/base.h"
#include "ivx/core/logger.h"
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ivx {
namespace {
static std::string read_file(const std::filesystem::path &filepath) {
    IVX_PROFILE_FUNCTION();

    std::ifstream file(filepath.string(), std::ios::in | std::ios::binary);
    if (!file) {
        IVX_CORE_ERROR("Failed to open shader file: {}", filepath.string());
    }

    std::stringstream stream;
    stream << file.rdbuf();

    if (file.bad()) {
        IVX_CORE_ERROR("Failed to read shader file: {}", filepath.string());
    }

    return stream.str();
}

static void
compile_and_link_shader(GLuint program,
                        const std::unordered_map<GLenum, std::string> &mp) {
    IVX_PROFILE_FUNCTION();

    std::vector<GLenum> shader_ids;
    shader_ids.reserve(mp.size());

    for (auto &&p : mp) {
        auto k = p.first;
        const auto &v = p.second;

        GLuint shader = glCreateShader(k);
        const GLchar *source = reinterpret_cast<const GLchar *>(v.c_str());
        glShaderSource(shader, 1, &source, 0);

        glCompileShader(shader);

        GLint is_compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
        if (is_compiled == GL_FALSE) {
            GLint max_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

            std::vector<GLchar> info_log(max_length);
            glGetShaderInfoLog(shader, max_length, &max_length, &info_log[0]);

            glDeleteShader(shader);

            IVX_CORE_ERROR("{}", info_log.data());
            IVX_CORE_ASSERT(false, "Vertex shader compile failure!");

            break;
        }
        glAttachShader(program, shader);
        shader_ids.push_back(shader);
    }

    glLinkProgram(program);

    GLint is_linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&is_linked);
    if (is_linked == GL_FALSE) {
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> infoLog(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &infoLog[0]);

        glDeleteProgram(program);

        for (auto shader : shader_ids) {
            glDeleteShader(shader);
        }

        IVX_CORE_ERROR("{}", infoLog.data());
        IVX_CORE_ASSERT(false, "Program link failure!");
        return;
    }

    // Always detach shaders after a successful link.
    for (auto shader : shader_ids) {
        glDetachShader(program, shader);
    }
}

static GLenum shader_type_from_string(const std::string &type) {
    if (type == "vertex") {
        return GL_VERTEX_SHADER;
    } else if (type == "fragment" || type == "pixel") {
        return GL_FRAGMENT_SHADER;
    }

    IVX_CORE_ASSERT(false, "Unsupported shader type!");
    return 0;
}

static std::unordered_map<GLuint, std::string>
pre_process(const std::string &code) {
    IVX_PROFILE_FUNCTION();

    std::unordered_map<GLuint, std::string> mp;
    const std::string symbol{"#type"};
    std::remove_const_t<decltype(std::string::npos)> begin = 0;
    auto pos = code.find(symbol, begin);
    while (pos != std::string::npos) {
        auto crlf = code.find_first_of("\r\n", pos + 1);
        IVX_CORE_ASSERT(crlf != std::string::npos, "Syntax error");
        std::string type = code.substr(pos + symbol.size() + 1,
                                       crlf - (pos + symbol.size() + 1));
        IVX_CORE_ASSERT(shader_type_from_string(type),
                        "Invalid shader type specifier");
        begin = code.find_first_not_of("\r\n", crlf);
        pos = code.find(symbol, begin);

        std::string src;
        if (pos != std::string::npos) {
            src = code.substr(begin, pos - begin);
        } else {
            src = code.substr(begin);
        }
        mp[shader_type_from_string(type)] = src;
    }

    return mp;
}
} // namespace

OpenGLShader::OpenGLShader(const std::filesystem::path &filepath,
                           const std::string &label) {
    IVX_PROFILE_FUNCTION();

    label_ = label.empty() ? filepath.stem().string() : label;
    std::string src = read_file(filepath);
    std::unordered_map<GLuint, std::string> mp = pre_process(src);

    GLuint program = glCreateProgram();
    program_ = program;

    compile_and_link_shader(program, mp);

    // IVX_CORE_VERBOSE("{}", filepath.stem().string());
}

OpenGLShader::OpenGLShader(const std::string &vs_src,
                           const std::string &fs_src,
                           const std::string &label)
    : label_(label) {
    IVX_PROFILE_FUNCTION();

    GLuint program = glCreateProgram();
    program_ = program;

    std::unordered_map<GLenum, std::string> mp{{GL_VERTEX_SHADER, vs_src},
                                               {GL_FRAGMENT_SHADER, fs_src}};

    compile_and_link_shader(program, mp);
}

OpenGLShader::~OpenGLShader() {
    IVX_PROFILE_FUNCTION();

    glDeleteProgram(program_);
}

void OpenGLShader::bind() {
    IVX_PROFILE_FUNCTION();

    glUseProgram(program_);
}

void OpenGLShader::unbind() {
    IVX_PROFILE_FUNCTION();

    glUseProgram(0);
}

int OpenGLShader::location(const std::string &name) {
    if (auto it = uniform_mp_.find(name); it != uniform_mp_.end()) {
        return it->second;
    }

    auto loc = glGetUniformLocation(program_, name.c_str());
    if (loc == -1) {
        IVX_CORE_WARN("Uniform '{}' not found in shader '{}'", name, label_);
    }

    uniform_mp_[name] = loc;
    return loc;
}

void OpenGLShader::set(const std::string &name, bool val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform1i(program_, location(name), static_cast<GLint>(val));
}

void OpenGLShader::set(const std::string &name, float val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform1f(program_, location(name), val);
}

void OpenGLShader::set(const std::string &name, int val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform1i(program_, location(name), val);
}

void OpenGLShader::set(const std::string &name, const glm::vec2 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform2fv(program_, location(name), 1, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::vec3 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform3fv(program_, location(name), 1, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::vec4 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform4fv(program_, location(name), 1, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::ivec2 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform2iv(program_, location(name), 1, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::ivec3 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform3iv(program_, location(name), 1, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::ivec4 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform4iv(program_, location(name), 1, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::mat3 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniformMatrix3fv(
        program_, location(name), 1, GL_FALSE, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name, const glm::mat4 &val) {
    IVX_PROFILE_FUNCTION();

    glProgramUniformMatrix4fv(
        program_, location(name), 1, GL_FALSE, glm::value_ptr(val));
}

void OpenGLShader::set(const std::string &name,
                       const int *vals,
                       uint32_t count) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform1iv(program_, location(name), count, vals);
}

void OpenGLShader::set(const std::string &name,
                       const float *vals,
                       uint32_t count) {
    IVX_PROFILE_FUNCTION();

    glProgramUniform1fv(program_, location(name), count, vals);
}

} // namespace ivx
