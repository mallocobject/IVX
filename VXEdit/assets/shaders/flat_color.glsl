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

struct Object {
    vec4 color;
};

uniform Object u_Object;

layout(location = 0) out vec4 v_Color;

void main() {
    v_Color = u_Object.color;
    gl_Position = camera.view_projection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec4 v_Color;

void main() {
    color = v_Color;
}
