#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform vec4 u_Color;
uniform mat4 u_VP;

out vec4 o_Color;

void main() {
    o_Color = u_Color;
    gl_Position = u_VP * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec4 o_Color;

void main() {
    color = o_Color;
}