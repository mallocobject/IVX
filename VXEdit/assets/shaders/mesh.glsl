#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_VP;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    v_Normal = u_NormalMatrix * a_Normal;
    v_TexCoord = a_TexCoord;
    gl_Position = u_VP * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out uint uid;

in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec3 u_LightDirection;
uniform vec4 u_Color;
uniform uint u_Uid;

void main() {
    float ndotl = max(dot(normalize(v_Normal), -normalize(u_LightDirection)), 0.0);
    float ambient = 0.35;
    color = vec4(u_Color.rgb * (ambient + (1.0 - ambient) * ndotl), u_Color.a);
    uid = u_Uid;
}
