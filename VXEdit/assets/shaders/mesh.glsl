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

const int kMaxLights = 8;

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

// Unit 0; a missing map binds a white 1x1 texture, so no branch is needed.
layout(binding = 0) uniform sampler2D u_AlbedoMap;

struct Light {
    vec4 position_type;    // xyz position, w type (0 directional / 1 point / 2 spot)
    vec4 direction_linear; // xyz direction the light travels in, w linear falloff
    vec4 color_intensity;  // rgb color, w intensity
    vec4 falloff;          // x constant, y linear, z quadratic
    vec4 spot_angles;      // x inner cos, y outer cos
};

// std140, 672 bytes. Mirrors GpuLightData in light_uniforms.h.
layout(std140, binding = 1) uniform LightBlock {
    vec4 ambient_exposure; // rgb ambient, w exposure
    ivec4 counts;          // x number of lights
    Light lights[kMaxLights];
};

void main() {
    vec3 albedo =
        texture(u_AlbedoMap, v_TexCoord * u_Object.tiling).rgb * u_Object.albedo.rgb;
    vec3 n = normalize(v_Normal);

    vec3 diffuse = vec3(0.0);
    for (int i = 0; i < counts.x; ++i) {
        Light light = lights[i];
        vec3 position = light.position_type.xyz;
        vec3 direction = light.direction_linear.xyz;
        float type = light.position_type.w;

        vec3 to_light;
        float attenuation = 1.0;

        if (type < 0.5) {
            // Directional: no falloff, the direction is all that matters.
            to_light = -normalize(direction);
        } else {
            vec3 delta = position - v_WorldPosition;
            float distance = length(delta);
            to_light = delta / max(distance, 1e-4);

            // Physical falloff: 1 / (constant + linear * d + quadratic * d^2). The coefficients
            // carry the world unit, so a scene measured in hundreds of units needs a quadratic
            // near 1e-5 rather than 1.
            float denominator = light.falloff.x +
                                light.falloff.y * distance +
                                light.falloff.z * distance * distance;
            attenuation = 1.0 / max(denominator, 1e-4);

            if (type > 1.5) {
                float cos_angle = dot(-to_light, normalize(direction));
                attenuation *= smoothstep(light.spot_angles.y, light.spot_angles.x, cos_angle);
            }
        }

        float ndotl = max(dot(n, to_light), 0.0);
        diffuse += light.color_intensity.rgb * light.color_intensity.w * ndotl * attenuation;
    }

    // Ambient is the floor, lights add on top of it scaled by the room the ambient leaves.
    // A single white light gives ambient + (1 - ambient) * N.L, which is the old formula, while
    // coloured lights tint the surface instead of washing it towards white. Sums can exceed 1
    // until an HDR target and tonemapping exist.
    vec3 lit = albedo * (ambient_exposure.rgb + (1.0 - ambient_exposure.rgb) * diffuse);
    lit *= ambient_exposure.w;

    // Albedo maps are sampled as sRGB, so lighting is linear; encode back for an RGBA8 target.
    color = vec4(pow(lit, vec3(1.0 / 2.2)), u_Object.albedo.a);
    uid = u_Object.uid;
}
