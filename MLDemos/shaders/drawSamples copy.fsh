#version 120

varying vec2 screenCoord;
varying float sampleColor;

const int colorCnt = 22;
void main()
{
	vec2 pos = screenCoord - vec2(0.5);
    float radius = dot(pos,pos);
    if(radius > 0.25) discard;
    if(radius > 0.12) gl_FragColor = vec4(0,0,0,1.);
    else
    {
        float s = sampleColor;
        while(s > colorCnt) s -= colorCnt;
        if (s < .9) gl_FragColor = vec4(1,1,1,1);
        else if (s < 1.9) gl_FragColor = vec4(1,0,0,1);
        else if (s < 2.9) gl_FragColor = vec4(0,1,0,1);
        else if (s < 3.9) gl_FragColor = vec4(0,0,1,1);
        else if (s < 4.9) gl_FragColor = vec4(1,1,0,1);
        else if (s < 5.9) gl_FragColor = vec4(1,0,1,1);
        else if (s < 6.9) gl_FragColor = vec4(0,1,1,1);
        else if (s < 7.9) gl_FragColor = vec4(1,.5,0,1);
        else if (s < 8.9) gl_FragColor = vec4(1,0,.5,1);
        else if (s < 9.9) gl_FragColor = vec4(0,1,.5,1);
        else if (s < 10.9) gl_FragColor = vec4(.5,1,0,1);
        else if (s < 11.9) gl_FragColor = vec4(.5,0,1,1);
        else if (s < 12.9) gl_FragColor = vec4(0,.5,1,1);
        else if (s < 13.9) gl_FragColor = vec4(.5,.5,.5,1);
        else if (s < 14.9) gl_FragColor = vec4(.31,.31,.31,1);
        else if (s < 15.9) gl_FragColor = vec4(0,.5,.31,1);
        else if (s < 16.9) gl_FragColor = vec4(1,.31,0,1);
        else if (s < 17.9) gl_FragColor = vec4(1,0,.31,1);
        else if (s < 18.9) gl_FragColor = vec4(0,1,.31,1);
        else if (s < 19.9) gl_FragColor = vec4(.31,1,0,1);
        else if (s < 20.9) gl_FragColor = vec4(.31,0,1,1);
        else if (s < 21.9) gl_FragColor = vec4(0,.31,1,1);
    }
}
