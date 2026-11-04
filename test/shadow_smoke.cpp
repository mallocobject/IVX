#include "ivx/core/window.h"
#include "ivx/render/renderer3d.h"
#include "ivx/render/shadow_pass.h"
#include "ivx/utils/math.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

static void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

class TestCamera : public ivx::Camera {
  public:
    glm::vec3 eye{8.f, 8.f, 12.f};
    glm::mat4 get_view_matrix() const override {
        return glm::lookAt(eye, glm::vec3{0.f}, glm::vec3{0.f, 1.f, 0.f});
    }
    glm::mat4 get_proj_matrix() const override {
        return ivx::perspective_rh_reverse_z(
            glm::radians(60.f), 1.6f, get_near(), get_far());
    }
    glm::vec3 get_position() const override {
        return eye;
    }
    float get_near() const override {
        return 0.1f;
    }
    float get_far() const override {
        return 1000.f;
    }
};

int main() {
    try {
        auto window = ivx::Window::create({"Shadow regression", 64, 64});
        SDL_HideWindow(static_cast<SDL_Window *>(window->get_native_window()));
        ivx::RenderCommand::init();
        ivx::Renderer3D::init(); // Also compiles the production lit/depth
                                 // shaders.
        {
            // Execute the actual production shadow functions with controlled
            // world positions. This avoids camera occlusion masking a failure.
            std::ifstream file(TEST_SHADER_DIR "mesh.glsl");
            std::stringstream input;
            input << file.rdbuf();
            std::string fragment = input.str();
            const auto stage = fragment.find("#type fragment");
            require(stage != std::string::npos, "Missing fragment stage");
            fragment = fragment.substr(fragment.find("#version", stage));
            fragment.resize(fragment.find("void main()"));
            fragment += R"(
uniform vec3 u_World;
uniform vec3 u_ToLight;
void main() {
    float v = shadow_factor(u_World, vec3(0, 1, 0), u_ToLight);
    color = vec4(v, v, v, 1);
    uid = 0u;
})";
            const std::string vertex = R"(#version 450 core
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
})";
            auto probe =
                ivx::Shader::create(vertex, fragment, "shadow regression");
            auto triangle = ivx::VertexArray::create();
            uint32_t indices[]{0, 1, 2};
            triangle->set_index_buffer(ivx::IndexBuffer::create(indices, 3));
            ivx::RenderScopeSpecification spec;
            ivx::FrameBufferSpecification target;
            target.width = target.height = 4;
            target.num_samples = 1;
            target.color_formats = {ivx::FrameBufferColorFormat::RGBA16F};
            spec.target = target;
            spec.depth_test = spec.depth_write = false;
            spec.blend.enabled = false;
            ivx::RenderScope output(spec);
            ivx::ShadowUniforms uniforms;
            auto sample = [&](const ivx::Ref<ivx::FrameBuffer> &map,
                              const ivx::GpuShadow &gpu,
                              glm::vec3 world,
                              glm::vec3 to_light) {
                uniforms.update(gpu);
                map->bind_depth_attachment(ivx::k_shadow_texture_unit);
                output.begin();
                probe->bind();
                probe->set("u_World", world);
                probe->set("u_ToLight", to_light);
                triangle->bind();
                ivx::RenderCommand::draw(triangle);
                output.end();
                float rgba[4]{};
                glGetTextureSubImage(output.target()->get_color_attachment(),
                                     0,
                                     1,
                                     1,
                                     0,
                                     1,
                                     1,
                                     1,
                                     GL_RGBA,
                                     GL_FLOAT,
                                     sizeof(rgba),
                                     rgba);
                require(glGetError() == GL_NO_ERROR, "OpenGL error");
                return rgba[0];
            };

            ivx::FrameBufferSpecification depth_spec;
            depth_spec.width = depth_spec.height = 32;
            depth_spec.num_samples = 1;
            depth_spec.depth_format = ivx::FrameBufferDepthFormat::Depth32F;
            auto depth = ivx::FrameBuffer::create(depth_spec);
            depth->bind();
            depth->clear({}, false, true);
            depth->unbind();
            require(sample(depth, ivx::GpuShadow{}, {0, 0, 0.5f}, {0, 1, 0}) >
                        0.99f,
                    "First frame without a light must be unshadowed");
            std::vector<float> pixels(32 * 32);
            for (int y = 0; y < 32; ++y)
                for (int x = 0; x < 32; ++x)
                    pixels[y * 32 + x] = x < 16 ? 0.8f : 0.2f;
            glTextureSubImage2D(depth->get_depth_attachment(),
                                0,
                                0,
                                0,
                                32,
                                32,
                                GL_DEPTH_COMPONENT,
                                GL_FLOAT,
                                pixels.data());
            ivx::GpuShadow gpu;
            gpu.params.x = 1;
            // Outside every light-frustum plane must stay lit, including z < 0.
            for (glm::vec3 world : {glm::vec3{0, 0, -0.1f},
                                    {0, 0, 1.1f},
                                    {-1.1f, 0, 0.5f},
                                    {1.1f, 0, 0.5f},
                                    {0, -1.1f, 0.5f},
                                    {0, 1.1f, 0.5f}})
                require(sample(depth, gpu, world, {0, 1, 0}) > 0.99f,
                        "Outside-frustum shadow regression");
            int transitions = 0;
            float previous = 0;
            for (int i = 0; i <= 100; ++i) {
                const float x = -0.12f + 0.24f * i / 100.f;
                float value = sample(depth, gpu, {x, 0, 0.5f}, {0, 1, 0});
                require(value + 0.002f >= previous, "PCF is not monotonic");
                if (value > previous + 0.001f)
                    ++transitions;
                previous = value;
            }
            require(transitions > 40,
                    "PCF still has discrete visibility bands");
            depth->bind();
            depth->clear({}, false, true);
            depth->unbind();
            require(sample(depth, gpu, {0.999f, 0, 0.5f}, {0, 1, 0}) > 0.99f,
                    "Border texels introduce false shadow");

            // Actual scene depth generation, while orbiting and pulling back.
            ivx::Scene scene;
            auto floor = scene.create_entity("Receiver");
            floor.add_component<ivx::MeshRendererComponent>().mesh =
                ivx::Mesh::create_cube(1);
            floor.get_component<ivx::TransformComponent>().scale = {
                200, 0.2f, 200};
            floor.get_component<ivx::TransformComponent>().translation.y =
                -0.1f;
            auto cube = scene.create_entity("Caster");
            cube.add_component<ivx::MeshRendererComponent>().mesh =
                ivx::Mesh::create_cube(2);
            cube.get_component<ivx::TransformComponent>().translation.y = 1;
            ivx::LightSet lights;
            lights.count = 1;
            lights.lights[0].properties.cast_shadows = true;
            lights.lights[0].direction =
                glm::normalize(glm::vec3{-0.4f, -1, -0.25f});
            const auto to_light = -lights.lights[0].direction;
            ivx::ShadowPass pass(1024);
            TestCamera camera;
            for (glm::vec3 eye : {glm::vec3{8, 8, 12},
                                  {-8, 8, -12},
                                  {16, 16, 24},
                                  {40, 40, 60},
                                  {80, 80, 120}}) {
                camera.eye = eye;
                require(pass.render(scene, lights, camera),
                        "Shadow pass failed");
                const float lit =
                    sample(pass.output(), pass.gpu(), {4, 0, 0}, to_light);
                std::cout << "eye=" << eye.x << ',' << eye.y << ',' << eye.z
                          << " lit=" << lit << " shadow="
                          << sample(pass.output(),
                                    pass.gpu(),
                                    {-1.4f, 0, -0.5f},
                                    to_light)
                          << '\n';
                require(lit > 0.95f,
                        "Lit ground became shadowed while zooming out");
                for (int x = 3; x <= 7; ++x)
                    for (int z = -3; z <= 3; ++z)
                        require(sample(pass.output(),
                                       pass.gpu(),
                                       {static_cast<float>(x),
                                        0,
                                        static_cast<float>(z)},
                                       to_light) > 0.95f,
                                "Self-shadow bands on the receiver plane");
                if (glm::length(eye) < 40.f)
                    require(sample(pass.output(),
                                   pass.gpu(),
                                   {-1.4f, 0, -0.5f},
                                   to_light) < 0.3f,
                            "Caster shadow missing");
            }
            lights.lights[0].properties.cast_shadows = false;
            require(!pass.render(scene, lights, camera) &&
                        pass.gpu().params.x == 0,
                    "Disabled light left stale shadows");
            require(
                sample(pass.output(), pass.gpu(), {-1.4f, 0, -0.5f}, to_light) >
                    0.99f,
                "Disabled light still darkens its former shadow");
            lights.lights[0].properties.cast_shadows = true;
            lights.lights[0].direction = glm::vec3{0};
            require(!pass.render(scene, lights, camera) &&
                        pass.gpu().params.x == 0,
                    "Invalid light direction produced a shadow matrix");
        }
        ivx::Renderer3D::shutdown();
        std::cout << "Shadow regression passed: frustum bounds, smooth PCF, "
                     "border, depth rendering, camera orbit/zoom, disable\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
