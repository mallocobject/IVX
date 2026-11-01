#type vertex
#version 450 core

struct Camera {
    mat4 view;
    mat4 projection;
    mat4 view_projection;
    vec4 position; // xyz
};

layout(std140, binding = 0) uniform CameraBlock {
    Camera camera;
};

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in uint a_Uid;

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) flat out int v_TexIndex;
layout(location = 3) out float v_TilingFactor;
layout(location = 4) flat out uint v_Uid;

void main() {
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
    v_Uid = a_Uid;
    gl_Position = camera.view_projection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out uint uid;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) flat in int v_TexIndex;
layout(location = 3) in float v_TilingFactor;
layout(location = 4) flat in uint v_Uid;

layout(binding = 0) uniform sampler2D u_Textures[32];

void main() {
    color = texture(u_Textures[v_TexIndex], v_TexCoord * v_TilingFactor) * v_Color;
    uid = v_Uid;
}
