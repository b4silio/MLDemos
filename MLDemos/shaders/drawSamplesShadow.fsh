#version 120

uniform sampler2D color_texture;
uniform sampler2D shadow_texture;
uniform mat4 lightMvpMatrix;
uniform mat4 lightMvMatrix;
uniform float pointSize;

const float Near = 1.0;
const float Far = 60.0;
const float Depth = 1.0 / (Far - Near);
const mat4 ScaleMatrix = mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
const float texSize = 512;

//varying float asd;
varying float sampleR;
varying float sampleG;
varying float sampleB;
varying float stuff;
varying float lvX;
varying float lvY;
varying float lvZ;
varying float lvW;

float unpack (vec4 colour)
{
    const vec4 bitShifts = vec4(1.0,
                    1.0 / 255.0,
                    1.0 / (255.0 * 255.0),
                    1.0 / (255.0 * 255.0 * 255.0));
    return dot(colour, bitShifts);
}

void main()
{
    vec2 pos = gl_PointCoord-vec2(0.5,0.5);
	float radius = dot(pos,pos);
    if(radius > 0.21) discard;
    vec4 lP = vec4(lvX, lvY, lvZ, lvW);
    lP = lightMvpMatrix * lP;
    lP = ScaleMatrix * lP;
    lP += vec4(pos*(pointSize/texSize),0,0);
    vec3 lPos = lP.xyz / lP.w;
    vec4 color = texture2D(color_texture, gl_PointCoord)*vec4(sampleR, sampleG, sampleB, 1);
    float z = lvZ / lvW;
    vec4 texel = texture2D(shadow_texture, lPos.xy);
    //float tZ = unpack(texel);
    float tZ = texel.g;
    if(lPos.z > tZ)
    {
        float c = 4.0;
        float shadow = clamp(exp(-c * (lPos.z - tZ)) + 0.5, 0.0, 1.0);
        gl_FragColor = color * shadow;
        gl_FragColor.a = color.a;
    }
    else gl_FragColor = color;

    return;
    float ss = stuff;
}
