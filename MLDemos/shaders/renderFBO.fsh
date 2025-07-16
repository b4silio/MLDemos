#version 120

uniform sampler2D texture;
uniform float counter;
varying vec2 texcoord;

void main(void) {
    vec2 tex = texcoord;
    //tex.x += sin(texcoord.y * 4*2*3.14159 + counter) / 100;
    gl_FragColor = texture2D(texture, tex);
}
