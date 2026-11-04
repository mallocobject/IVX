#include "ivxpch.h"
#include "scene_renderer.h"

namespace ivx {
namespace {
RenderScopeSpecification scene_specification(uint32_t width, uint32_t height) {
    FrameBufferSpecification target;
    target.width = width;
    target.height = height;
    target.num_samples = 4;
    target.color_formats = {FrameBufferColorFormat::RGBA16F,
                            FrameBufferColorFormat::RED_INTEGER};
    target.depth_format = FrameBufferDepthFormat::Depth24Stencil8;

    RenderScopeSpecification spec;
    spec.target = target;
    // Linear grey becomes approximately 0.1 after Reinhard and sRGB.
    spec.clear_color = {0.0129f, 0.0129f, 0.0129f, 1.f};
    spec.cull = RendererAPI::CullMode::IVX_Back;
    return spec;
}
} // namespace

SceneRenderer::SceneRenderer(uint32_t width, uint32_t height)
    : scene_scope_(scene_specification(width, height)), shadows_(2048),
      tone_mapping_(width, height) {
}

void SceneRenderer::resize(uint32_t width, uint32_t height) {
    scene_scope_.resize(width, height);
    tone_mapping_.resize(width, height);
}

void SceneRenderer::render(Scene *scene, const Camera &camera) {
    IVX_PROFILE_FUNCTION();
    if (scene) {
        scene->update_world_transforms();
    }
    const LightSet lights = scene ? scene->collect_lights() : LightSet{};
    const bool has_shadow = scene && shadows_.render(*scene, lights, camera);

    light_uniforms_.update(lights);
    shadow_uniforms_.update(has_shadow ? shadows_.gpu() : GpuShadowData{});
    shadows_.bind_maps();

    scene_scope_.begin(&camera);
    if (scene) {
        scene->render();
    }
    scene_scope_.end();

    const Environment environment =
        scene ? scene->environment() : Environment{};
    tone_mapping_.render(
        hdr_target(), environment.exposure, environment.tonemap);
}

uint32_t SceneRenderer::read_entity(uint32_t x, uint32_t y) const {
    return hdr_target()->read_pixel(1, x, y);
}
} // namespace ivx
