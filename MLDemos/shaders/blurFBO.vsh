#version 120

// adapted from
// http://xissburg.com/faster-gaussian-blur-in-glsl/

attribute vec2 vertex;
uniform sampler2D texture;
uniform bool bVertical;
uniform int amount;

varying vec2 texcoord;
varying vec2 blurTexCoords[14];

void main(void) {
    gl_Position = vec4(vertex, 0.0, 1.0);
    texcoord = (vertex + 1.0) / 2.0;
    const float step = 0.004;
    float v = step;
    if(bVertical)
    {
        if(amount > 0)
        {
            blurTexCoords[ 6] = texcoord + vec2(0.0, -v);
            blurTexCoords[ 7] = texcoord + vec2(0.0,  v);
        } else return;
        v += step;
        if(amount > 1)
        {
            blurTexCoords[ 5] = texcoord + vec2(0.0, -v);
            blurTexCoords[ 8] = texcoord + vec2(0.0,  v);
        } else return;
        v += step;
        if(amount > 2)
        {
            blurTexCoords[ 4] = texcoord + vec2(0.0, -v);
            blurTexCoords[ 9] = texcoord + vec2(0.0,  v);
        } else return;
        v += step;
        if(amount > 3)
        {
            blurTexCoords[ 3] = texcoord + vec2(0.0, -v);
            blurTexCoords[10] = texcoord + vec2(0.0,  v);
        } else return;
        v += step;
        if(amount > 4)
        {
            blurTexCoords[ 2] = texcoord + vec2(0.0, -v);
            blurTexCoords[11] = texcoord + vec2(0.0,  v);
        } else return;
        v += step;
        if(amount > 5)
        {
            blurTexCoords[ 1] = texcoord + vec2(0.0, -v);
            blurTexCoords[12] = texcoord + vec2(0.0,  v);
        } else return;
        v += step;
        if(amount > 6)
        {
            blurTexCoords[ 0] = texcoord + vec2(0.0, -v);
            blurTexCoords[13] = texcoord + vec2(0.0,  v);
        } else return;
    }
    else
    {
        if(amount > 0)
        {
            blurTexCoords[ 6] = texcoord + vec2(-v, 0.0);
            blurTexCoords[ 7] = texcoord + vec2( v, 0.0);
        } else return;
        v += step;
        if(amount > 1)
        {
            blurTexCoords[ 5] = texcoord + vec2(-v, 0.0);
            blurTexCoords[ 8] = texcoord + vec2( v, 0.0);
        } else return;
        v += step;
        if(amount > 2)
        {
            blurTexCoords[ 4] = texcoord + vec2(-v, 0.0);
            blurTexCoords[ 9] = texcoord + vec2( v, 0.0);
        } else return;
        v += step;
        if(amount > 3)
        {
            blurTexCoords[ 3] = texcoord + vec2(-v, 0.0);
            blurTexCoords[10] = texcoord + vec2( v, 0.0);
        } else return;
        v += step;
        if(amount > 4)
        {
            blurTexCoords[ 2] = texcoord + vec2(-v, 0.0);
            blurTexCoords[11] = texcoord + vec2( v, 0.0);
        } else return;
        v += step;
        if(amount > 5)
        {
            blurTexCoords[ 1] = texcoord + vec2(-v, 0.0);
            blurTexCoords[12] = texcoord + vec2( v, 0.0);
        } else return;
        v += step;
        if(amount > 6)
        {
            blurTexCoords[ 0] = texcoord + vec2(-v, 0.0);
            blurTexCoords[13] = texcoord + vec2( v, 0.0);
        } else return;
    }
}
