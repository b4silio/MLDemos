#version 120

uniform sampler2D color_texture;

varying float stuff;
varying float sampleR;
varying float sampleG;
varying float sampleB;

void main()
{
    vec2 pos = gl_PointCoord.xy-vec2(0.5,0.5);
	float radius = dot(pos,pos);
    if(radius > 0.21) discard;
    gl_FragColor = texture2D(color_texture, gl_PointCoord.xy)*vec4(sampleR, sampleG, sampleB, 1);
    return;
    float s = stuff;
//    gl_FragColor = texture2D(color_texture, gl_PointCoord.xy)*vec4(stuff, 0, 0, 1);
//    if(radius > 0.16) gl_FragColor = vec4(0,0,0,1.);
//    else gl_FragColor = vec4(sampleR, sampleG, sampleB, 1.);
}
