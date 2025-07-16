#version 120

attribute vec2 vertex;
uniform sampler2D texture;
//uniform float counter;

varying vec2 texcoord;

void main(void) {
    gl_Position = vec4(vertex, 0.0, 1.0);
    texcoord = (vertex + 1.0) / 2.0;
}
