#version 120

attribute vec3 position;
attribute vec3 normal;

uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;

varying vec3 vNormal;

void main() {
    vNormal = normalMatrix * normal;
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
