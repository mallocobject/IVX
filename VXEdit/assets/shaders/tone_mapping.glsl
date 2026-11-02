#type vertex
#version 450 core

layout(location = 0) out vec2 v_UV;

void main() {
    // Oversized full-screen triangle; no vertex buffer or diagonal seam.
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    v_UV = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) in vec2 v_UV;
layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D u_HDR;
uniform float u_Exposure;
uniform int u_Curve; // ToneMappingCurve: 0 Reinhard, 1 ACES, 2 Khronos Neutral

// Mirrored by tonemap_reinhard / tonemap_aces / tonemap_neutral in ivx/utils/math.h: the smoke
// test compares the two, so they have to stay identical.
vec3 reinhard(vec3 linear) {
    return linear / (vec3(1.0) + linear);
}

vec3 aces(vec3 linear) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp(linear * (a * linear + b) / (linear * (c * linear + d) + e), 0.0, 1.0);
}

vec3 neutral(vec3 linear) {
    const float start_compression = 0.8 - 0.04;
    const float desaturation = 0.15;

    vec3 color = linear;
    float darkest = min(color.r, min(color.g, color.b));
    float offset = darkest < 0.08 ? darkest - 6.25 * darkest * darkest : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < start_compression) {
        return max(color, vec3(0.0));
    }

    float d = 1.0 - start_compression;
    float new_peak = 1.0 - d * d / (peak + d - start_compression);
    color *= new_peak / peak;

    float whitepoint = 1.0 - 1.0 / (desaturation * (peak - new_peak) + 1.0);
    return max(mix(color, vec3(1.0), whitepoint), vec3(0.0));
}

// Exact linear-to-sRGB transfer, applied once, after tone mapping. The target is plain RGBA8,
// so the encoding has to happen here.
vec3 encode_srgb(vec3 linear) {
    return mix(1.055 * pow(linear, vec3(1.0 / 2.4)) - 0.055,
               linear * 12.92,
               lessThanEqual(linear, vec3(0.0031308)));
}

void main() {
    vec4 hdr = texture(u_HDR, v_UV);
    vec3 linear = max(hdr.rgb, vec3(0.0)) * u_Exposure;

    vec3 mapped = u_Curve == 1 ? aces(linear)
                 : u_Curve == 2 ? neutral(linear)
                                : reinhard(linear);

    color = vec4(encode_srgb(mapped), hdr.a);
}
