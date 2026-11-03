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

layout(binding = 0) uniform sampler2D u_AlbedoMap;

struct Light {
    vec4 position_type;    // xyz position, w type (0 directional / 1 point / 2
    vec4 direction_linear; // xyz direction the light travels in, w linear
    vec4 color_intensity;  // rgb color, w intensity
    vec4 falloff;          // x constant, y linear, z quadratic
    vec4 spot_angles;      // x inner cos, y outer cos
};

layout(std140, binding = 1) uniform LightBlock {
    vec4 ambient; // rgb ambient, w padding (exposure is the tonemap pass)
    ivec4 counts; // x number of lights
    Light lights[kMaxLights];
};

struct Shadow {
    mat4 light_space;
    vec4 params;  // x enabled, y base bias, z world texel size, w normal bias
    vec4 params2; // x slope scale
    ivec4 light_index;
};

layout(std140, binding = 2) uniform ShadowBlock {
    Shadow shadow;
};

layout(binding = 2) uniform sampler2D u_ShadowMap;

float is_in_shadow(vec2 uv, float reference) {
    return texture(u_ShadowMap, uv).r > reference ? 1.0 : 0.0;
}

float bilinear_shadow(vec2 uv, float reference, vec2 texelUV) {
    // Interpolate comparisons, not depth samples.
    vec2 pixel = uv / texelUV - 0.5;
    vec2 fraction = fract(pixel);
    vec2 base = (floor(pixel) + 0.5) * texelUV;
    float a = is_in_shadow(base, reference);
    float b = is_in_shadow(base + vec2(texelUV.x, 0.0), reference);
    float c = is_in_shadow(base + vec2(0.0, texelUV.y), reference);
    float d = is_in_shadow(base + texelUV, reference);
    return mix(mix(a, b, fraction.x), mix(c, d, fraction.x), fraction.y);
}

float shadow_calculation(vec3 world, vec3 n, vec3 to_light) {
    if (shadow.params.x < 0.5) {
        return 0.0;
    }

    float ndotl = clamp(dot(n, to_light), 0.0, 1.0);
    vec3 biased = world + n * shadow.params.z * shadow.params.w * (1.0 - ndotl);
    vec4 clip = shadow.light_space * vec4(biased, 1.0);
    if (clip.w <= 0.0) {
        return 0.0;
    }
    vec3 projected = clip.xyz / clip.w;
    // Zero-to-one reverse-Z: only XY needs remapping.
    vec2 uv = projected.xy * 0.5 + 0.5;

    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))) ||
        projected.z < 0.0 || projected.z > 1.0) {
        return 0.0;
    }

    float slope = sqrt(max(1.0 - ndotl * ndotl, 0.0)) / max(ndotl, 0.1);
    float bias = max(shadow.params.y, shadow.params2.x * slope);
    float reference = projected.z + bias;

    vec2 texelUV = 1.0 / vec2(textureSize(u_ShadowMap, 0));
    float visibility = 0.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            visibility += bilinear_shadow(uv + vec2(x, y) * texelUV,
                                          reference, texelUV);
        }
    }
    return visibility / 9.0;
}

void main() {
    vec3 albedo = texture(u_AlbedoMap, v_TexCoord * u_Object.tiling).rgb *
                  u_Object.albedo.rgb;
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
            to_light = -normalize(direction);
        } else {
            vec3 delta = position - v_WorldPosition;
            float distance = length(delta);
            to_light = delta / max(distance, 1e-4);

            float denominator = light.falloff.x + light.falloff.y * distance +
                                light.falloff.z * distance * distance;
            attenuation = 1.0 / max(denominator, 1e-4);

            if (type > 1.5) {
                float cos_angle = dot(-to_light, normalize(direction));
                attenuation *= smoothstep(
                    light.spot_angles.y, light.spot_angles.x, cos_angle);
            }
        }

        float visibility = (i == shadow.light_index.x)
                               ? (1 - shadow_calculation(v_WorldPosition, n, to_light))
                               : 1.0;

        float ndotl = max(dot(n, to_light), 0.0);
        diffuse += light.color_intensity.rgb * light.color_intensity.w * ndotl *
                   attenuation * visibility;
    }

    vec3 lit = albedo * (ambient.rgb + (1.0 - ambient.rgb) * diffuse);
    color = vec4(lit, u_Object.albedo.a);
    uid = u_Object.uid;
}
