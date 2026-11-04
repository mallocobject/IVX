#type vertex
#version 450 core

layout(location = 0) out vec2 v_NDC;

void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    v_NDC = p * 2.0 - 1.0;
    gl_Position = vec4(v_NDC, 0.0, 1.0);
}

#type fragment
#version 450 core

#include "lighting.glsl"

layout(location = 0) in vec2 v_NDC;
layout(location = 0) out vec4 color;
layout(location = 1) out uint uid;

uniform mat4 u_InverseVP;
uniform mat4 u_ViewProjection;
uniform float u_Height;
uniform float u_GridSpacing;
uniform vec3 u_Color;
uniform vec3 u_GridColor;

float grid(vec2 coord) {
    vec2 footprint = max(fwidth(coord), vec2(1e-6));
    vec2 distanceToLine = abs(fract(coord - 0.5) - 0.5) / footprint;
    vec2 coverage = 1.0 - smoothstep(vec2(0.0), vec2(1.0), distanceToLine);
    coverage *= 1.0 - smoothstep(vec2(0.25), vec2(1.0), footprint);
    return max(coverage.x, coverage.y);
}

void main() {
    vec4 a = u_InverseVP * vec4(v_NDC, 1.0, 1.0);
    vec4 b = u_InverseVP * vec4(v_NDC, 0.0, 1.0);
    vec3 origin = a.xyz / a.w;
    vec3 direction = b.xyz * a.w - a.xyz * b.w;
    if (abs(direction.y) < 1e-7) discard;
    float t = (u_Height - origin.y) / direction.y;
    if (t < 0.0) discard;
    vec3 world = origin + t * direction;
    vec4 clip = u_ViewProjection * vec4(world, 1.0);
    float depth = clip.z / clip.w;
    if (clip.w <= 0.0 || depth <= 0.0 || depth > 1.0) discard;

    vec2 coord = world.xz / u_GridSpacing;
    float coverage = max(grid(coord), grid(coord / 10.0) * 0.65);
    vec3 albedo = mix(u_Color, u_GridColor, coverage);
    vec3 normal = vec3(0.0, direction.y < 0.0 ? 1.0 : -1.0, 0.0);
    color = vec4(evaluate_lighting(world, normal, albedo), 1.0);
    uid = 0u;
    gl_FragDepth = depth;
}
