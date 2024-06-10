#version 450 core

in vec2 TexCoord;
out vec4 _FragColor;

uniform vec3 color;
uniform int wireframe;

void main() {
    float eps = 0.01;

    if(TexCoord.x < eps || TexCoord.x > 1 - eps || TexCoord.y < eps || TexCoord.y > 1 - eps) {
        _FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else if(wireframe != 0) {
        discard;
    } else {
        _FragColor = vec4(color, 1.0);
    }
}
