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
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

struct Object {
    mat4 model;
    mat3 normal_matrix;
    vec4 albedo;
    float tiling;
    uint uid;
};

uniform Object u_Object;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out vec3 v_WorldPosition;

void main() {
    vec4 world = u_Object.model * vec4(a_Position, 1.0);

    v_Normal = u_Object.normal_matrix * a_Normal;
    v_TexCoord = a_TexCoord;
    v_WorldPosition = world.xyz;
    gl_Position = camera.view_projection * world;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out uint uid;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in vec3 v_WorldPosition;

struct Object {
    mat4 model;
    mat3 normal_matrix;
    vec4 albedo;
    float tiling;
    uint uid;
};

uniform Object u_Object;

layout(binding = 0) uniform sampler2D u_AlbedoMap;

#include "lighting.glsl"

void main() {
    vec3 albedo = texture(u_AlbedoMap, v_TexCoord * u_Object.tiling).rgb *
                  u_Object.albedo.rgb;
    vec3 n = normalize(v_Normal);

    vec3 lit = evaluate_lighting(v_WorldPosition, n, albedo);
    color = vec4(lit, u_Object.albedo.a);
    uid = u_Object.uid;
}
