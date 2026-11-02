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

const int kMaxLights = 8;

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

struct Light {
    vec4 position_type;   // xyz position, w type (0 directional / 1 point / 2 spot)
    vec4 direction_range; // xyz direction the light travels in, w range
    vec4 color_intensity; // rgb color, w intensity
    vec4 spot_angles;     // x inner cos, y outer cos
};

// std140, 544 bytes. Mirrors GpuLightData in light_uniforms.h.
layout(std140, binding = 1) uniform LightBlock {
    vec4 ambient_exposure; // rgb ambient, w exposure
    ivec4 counts;          // x number of lights
    Light lights[kMaxLights];
};

void main() {
    vec3 albedo =
        texture(u_AlbedoMap, v_TexCoord * u_Object.tiling).rgb * u_Object.albedo.rgb;
    vec3 n = normalize(v_Normal);

    // Only directional lights so far: the entity light types land with LightComponent.
    vec3 diffuse = vec3(0.0);
    for (int i = 0; i < counts.x; ++i) {
        vec3 to_light = -normalize(lights[i].direction_range.xyz);
        float ndotl = max(dot(n, to_light), 0.0);
        diffuse += lights[i].color_intensity.rgb * lights[i].color_intensity.w * ndotl;
    }

    // Ambient is the floor and the diffuse sum pushes towards full brightness, which keeps the
    // single light case identical to the old per-shader uniforms.
    vec3 lit = albedo * mix(ambient_exposure.rgb, vec3(1.0), clamp(diffuse, 0.0, 1.0));
    lit *= ambient_exposure.w;

    // Albedo maps are sampled as sRGB, so lighting is linear; encode back for an RGBA8 target.
    color = vec4(pow(lit, vec3(1.0 / 2.2)), u_Object.albedo.a);
    uid = u_Object.uid;
}
