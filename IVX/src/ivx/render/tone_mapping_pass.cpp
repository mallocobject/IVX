#include "ivxpch.h"
#include "tone_mapping_pass.h"

#include <algorithm>

namespace ivx {
ToneMappingPass::ToneMappingPass(uint32_t width, uint32_t height) {
    FrameBufferSpecification spec;
    spec.width = width;
    spec.height = height;
    spec.num_samples = 1;
    // Plain RGBA8 on purpose: the shader already encodes sRGB, so an
    // SRGB8_ALPHA8 attachment would encode a second time.
    spec.attachments = {{FrameBufferTextureFormat::RGBA8}};
    output_ = FrameBuffer::create(spec);

    RenderPassSpecification pass;
    pass.target = output_;
    pass.depth_test = false;
    pass.depth_write = false;
    pass.blend.enabled = false;
    pass.cull = RendererAPI::CullMode::IVX_None;
    pass.clear_depth = false;
    pass.clear_color_attachments = false;
    pass_ = make_ref<RenderPass>(pass);
    shader_ = Shader::create(SHADERS "tone_mapping.glsl");
    triangle_ = VertexArray::create();
    uint32_t indices[]{0, 1, 2};
    triangle_->set_index_buffer(IndexBuffer::create(indices, 3));
}

void ToneMappingPass::resize(uint32_t width, uint32_t height) {
    if (width > 0 && height > 0) {
        output_->resize(width, height);
    }
}

void ToneMappingPass::render(const Ref<FrameBuffer> &source,
                             float exposure,
                             ToneMappingCurve curve) {
    IVX_PROFILE_FUNCTION();

    // A collapsed viewport leaves the output at zero size; there is nothing to
    // write then.
    const FrameBufferSpecification &spec = output_->get_specification();
    if (!source || spec.width == 0 || spec.height == 0) {
        return;
    }

    pass_->begin();
    shader_->bind();
    shader_->set("u_Exposure", std::max(exposure, 0.f));
    shader_->set("u_Curve", static_cast<int>(curve));
    source->bind_color_attachment(0);
    triangle_->bind();
    RenderCommand::draw(triangle_);
    pass_->end();
}
} // namespace ivx
