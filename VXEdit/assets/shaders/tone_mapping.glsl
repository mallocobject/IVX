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

void main() {
    vec4 hdr = texture(u_HDR, v_UV);
    vec3 linear = max(hdr.rgb, vec3(0.0)) * u_Exposure;
    // Reinhard compresses HDR highlights smoothly into the display range.
    vec3 mapped = linear / (vec3(1.0) + linear);
    // Exact linear-to-sRGB transfer, applied once, after tone mapping.
    vec3 srgb = mix(1.055 * pow(mapped, vec3(1.0 / 2.4)) - 0.055,
                    mapped * 12.92, lessThanEqual(mapped, vec3(0.0031308)));
    color = vec4(srgb, hdr.a);
}
