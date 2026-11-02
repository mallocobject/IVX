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

uniform mat4 u_Model;

void main() {
    // Depth only: the fragment stage writes nothing, so this is the whole pipeline.
    gl_Position = camera.view_projection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main() {
}
