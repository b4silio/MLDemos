#version 120

uniform vec4 viewport;
uniform sampler2D color_texture;

varying vec2 screenCoord;
varying float sampleR;
varying float sampleG;
varying float sampleB;
void main()
{
    vec2 pos = screenCoord - 0.5;
    float radius = dot(pos,pos);
    if(radius > 0.21) discard;
    gl_FragColor = texture2D(color_texture, screenCoord)*vec4(sampleR, sampleG, sampleB, 1);
//    if(radius > 0.16) gl_FragColor = vec4(0,0,0,1.);
//    else gl_FragColor = vec4(sampleR, sampleG, sampleB, 1.);
}
