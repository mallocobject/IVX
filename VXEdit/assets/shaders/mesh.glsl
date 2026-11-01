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

// Per object state, declared identically in both stages.
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

void main() {
    v_Normal = u_Object.normal_matrix * a_Normal;
    v_TexCoord = a_TexCoord;
    gl_Position = camera.view_projection * u_Object.model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out uint uid;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;

struct Object {
    mat4 model;
    mat3 normal_matrix;
    vec4 albedo;
    float tiling;
    uint uid;
};

uniform Object u_Object;

// Unit 0; a missing map binds a white 1x1 texture, so no branch is needed.
layout(binding = 0) uniform sampler2D u_AlbedoMap;

struct DirectionalLight {
    vec3 direction; // direction the light travels in
    float ambient;
};

uniform DirectionalLight u_Light;

void main() {
    vec3 albedo =
        texture(u_AlbedoMap, v_TexCoord * u_Object.tiling).rgb * u_Object.albedo.rgb;
    float ndotl = max(dot(normalize(v_Normal), -normalize(u_Light.direction)), 0.0);
    float ambient = clamp(u_Light.ambient, 0.0, 1.0);
    vec3 lit = albedo * (ambient + (1.0 - ambient) * ndotl);

    // Albedo maps are sampled as sRGB, so lighting is linear; encode back for an RGBA8 target.
    color = vec4(pow(lit, vec3(1.0 / 2.2)), u_Object.albedo.a);
    uid = u_Object.uid;
}
