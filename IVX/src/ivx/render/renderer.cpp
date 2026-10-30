#include "ivxpch.h"
#include "renderer.h"

#include "ivx/render/render_command.h"
#include "renderer2d.h"
#include "renderer3d.h"

namespace ivx {

void Renderer::init() {
    IVX_PROFILE_FUNCTION();

    RenderCommand::init();
    Renderer2D::init();
    Renderer3D::init();
}

void Renderer::shutdown() {
    IVX_PROFILE_FUNCTION();

    Renderer3D::shutdown();
    Renderer2D::shutdown();
}

void Renderer::on_window_resize(WindowResizeEvent &event) {
    RenderCommand::set_viewport(0, 0, event.get_width(), event.get_height());
}

} // namespace ivx
