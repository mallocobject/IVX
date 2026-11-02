#include "ivxpch.h"
#include "tone_mapping_pass.h"

#include <algorithm>

namespace ivx {
ToneMappingPass::ToneMappingPass(uint32_t width, uint32_t height) {
    RenderScopeSpecification scope_spec;

    FrameBufferSpecification target;
    target.width = width;
    target.height = height;
    target.num_samples = 1;
    // Plain RGBA8 on purpose: the shader already encodes sRGB, so an
    // SRGB8_ALPHA8 attachment would encode a second time. No depth attachment:
    // a full screen triangle does not need one.
    target.color_formats = {FrameBufferColorFormat::RGBA8};

    scope_spec.target = target;
    scope_spec.depth_test = false;
    scope_spec.depth_write = false;
    scope_spec.blend.enabled = false;
    scope_spec.cull = RendererAPI::CullMode::IVX_None;
    scope_spec.clear_depth = false;
    scope_spec.clear_color_attachments = false;
    scope_ = make_ref<RenderScope>(std::move(scope_spec));

    shader_ = Shader::create(SHADERS "tone_mapping.glsl");
    triangle_ = VertexArray::create();
    uint32_t indices[]{0, 1, 2};
    triangle_->set_index_buffer(IndexBuffer::create(indices, 3));
}

void ToneMappingPass::resize(uint32_t width, uint32_t height) {
    scope_->resize(width, height);
}

void ToneMappingPass::render(const Ref<FrameBuffer> &source,
                             float exposure,
                             ToneMappingCurve curve) {
    IVX_PROFILE_FUNCTION();

    // A collapsed viewport leaves the output at zero size; there is nothing to
    // write then.
    const FrameBufferSpecification &spec =
        scope_->target()->get_specification();
    if (!source || spec.width == 0 || spec.height == 0) {
        return;
    }

    scope_->begin();
    shader_->bind();
    shader_->set("u_Exposure", std::max(exposure, 0.f));
    shader_->set("u_Curve", static_cast<int>(curve));
    source->bind_color_attachment(0);
    triangle_->bind();
    RenderCommand::draw(triangle_);
    scope_->end();
}
} // namespace ivx
