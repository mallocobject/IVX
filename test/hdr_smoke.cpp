#include "ivx/core/window.h"
#include "ivx/render/tone_mapping_pass.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

// Reference path: the curve from ivx/utils/math.h followed by the sRGB
// transfer. The shader has to agree channel by channel, which is what the
// comparisons below enforce.
static float
expected(ivx::ToneMappingCurve curve, float radiance, float exposure) {
    const glm::vec3 linear = glm::vec3{radiance} * exposure;
    return ivx::encode_srgb(ivx::tonemap(curve, linear)).r;
}

int main() {
    try {
        auto window = ivx::Window::create({"HDR validation", 64, 64});
        SDL_HideWindow(static_cast<SDL_Window *>(window->get_native_window()));
        ivx::RenderCommand::init();
        ivx::ToneMappingPass tone(16, 16);
        for (uint32_t samples : {1u, 4u}) {
            ivx::FrameBufferSpecification spec;
            spec.width = 16;
            spec.height = 16;
            spec.num_samples = samples;
            spec.attachments = {{ivx::FrameBufferTextureFormat::RGBA16F},
                                {ivx::FrameBufferTextureFormat::RED_INTEGER}};
            auto source = ivx::FrameBuffer::create(spec);
            for (uint32_t size : {16u, 23u}) {
                source->resize(size, size);
                tone.resize(size, size);
                source->bind();
                source->clear({0.18f, 2.f, 16.f, 1.f});
                GLint fbo = 0;
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);
                const GLuint id[4]{42, 0, 0, 0};
                glClearNamedFramebufferuiv(fbo, GL_COLOR, 1, id);
                source->unbind();
                float hdr[4]{};
                glGetTextureSubImage(source->get_color_attachment(),
                                     0,
                                     0,
                                     0,
                                     0,
                                     1,
                                     1,
                                     1,
                                     GL_RGBA,
                                     GL_FLOAT,
                                     sizeof(hdr),
                                     hdr);
                require(std::abs(hdr[2] - 16.f) < 0.01f,
                        "HDR radiance clipped");

                for (ivx::ToneMappingCurve curve :
                     {ivx::ToneMappingCurve::IVX_Reinhard,
                      ivx::ToneMappingCurve::IVX_ACES,
                      ivx::ToneMappingCurve::IVX_Neutral}) {
                    for (float exposure : {0.f, 0.5f, 1.f, 4.f}) {
                        const auto state = ivx::RenderCommand::state();
                        GLint viewport[4], binding;
                        glGetIntegerv(GL_VIEWPORT, viewport);
                        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &binding);
                        tone.render(source, exposure, curve);
                        unsigned char rgba[4]{};
                        glGetTextureSubImage(
                            tone.output()->get_color_attachment(),
                            0,
                            size - 1,
                            size - 1,
                            0,
                            1,
                            1,
                            1,
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            sizeof(rgba),
                            rgba);
                        for (int c = 0; c < 3; ++c) {
                            const float want = expected(curve, hdr[c], exposure);
                            if (std::abs(rgba[c] / 255.f - want) >=
                                2.f / 255.f) {
                                std::cerr
                                    << "MISMATCH curve="
                                    << static_cast<int>(curve)
                                    << " exposure=" << exposure
                                    << " channel=" << c << " want=" << want
                                    << " got=" << rgba[c] / 255.f << '\n';
                            }
                            require(std::abs(rgba[c] / 255.f - want) <
                                        2.f / 255.f,
                                    "Incorrect tone mapping or sRGB encoding");
                        }
                        require(rgba[3] == 255, "Alpha changed");
                        require(source->read_pixel(1, size - 1, size - 1) == 42,
                                "Picking ID changed");
                        const auto restored = ivx::RenderCommand::state();
                        require(state.depth_test == restored.depth_test &&
                                    state.depth_write == restored.depth_write &&
                                    state.blend == restored.blend &&
                                    state.cull_faces == restored.cull_faces,
                                "Pipeline state not restored");
                        GLint after[4], after_binding;
                        glGetIntegerv(GL_VIEWPORT, after);
                        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,
                                      &after_binding);
                        require(binding == after_binding,
                                "Framebuffer not restored");
                        for (int c = 0; c < 4; ++c)
                            require(viewport[c] == after[c],
                                    "Viewport not restored");
                        require(glGetError() == GL_NO_ERROR, "OpenGL error");
                    }
                }
            }
        }
        std::cout
            << "HDR smoke passed: radiance, three curves, exposure, sRGB, "
               "MSAA, resize, IDs, state\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
