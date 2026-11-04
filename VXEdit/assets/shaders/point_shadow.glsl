#type vertex
#version 450 core

struct Camera {
    mat4 view;
    mat4 projection;
    mat4 view_projection;
    vec4 position;
};
layout(std140, binding = 0) uniform CameraBlock { Camera camera; };
layout(location = 0) in vec3 a_Position;
layout(location = 0) out vec3 v_WorldPosition;
uniform mat4 u_Model;

void main() {
    vec4 world = u_Model * vec4(a_Position, 1.0);
    v_WorldPosition = world.xyz;
    gl_Position = camera.view_projection * world;
}

#type fragment
#version 450 core

layout(location = 0) in vec3 v_WorldPosition;
uniform vec3 u_LightPosition;
uniform float u_Near;
uniform float u_Far;

void main() {
    float distanceToLight = length(v_WorldPosition - u_LightPosition);
    if (distanceToLight < u_Near || distanceToLight >= u_Far) discard;
    gl_FragDepth = (u_Far - distanceToLight) / (u_Far - u_Near);
}
