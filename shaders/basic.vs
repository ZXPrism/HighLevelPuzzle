#version 450 core

layout(location = 0) in vec3 _Coord;
layout(location = 1) in vec2 _TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 TexCoord;

void main() {
    TexCoord = _TexCoord;
    gl_Position = proj * view * model * vec4(_Coord, 1.0);
}
