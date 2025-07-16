#version 120

//precision mediump float;

uniform sampler2D texture;
uniform int amount;

varying vec2 texcoord;
varying vec2 blurTexCoords[14];

void main()
{
    gl_FragColor = vec4(0.0);
    if(amount > 6)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 0])*0.0044299121055113265;
        gl_FragColor += texture2D(texture, blurTexCoords[ 1])*0.00895781211794;
        gl_FragColor += texture2D(texture, blurTexCoords[ 2])*0.0215963866053;
        gl_FragColor += texture2D(texture, blurTexCoords[ 3])*0.0443683338718;
        gl_FragColor += texture2D(texture, blurTexCoords[ 4])*0.0776744219933;
        gl_FragColor += texture2D(texture, blurTexCoords[ 5])*0.115876621105;
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.147308056121;
        gl_FragColor += texture2D(texture, texcoord         )*0.159576912161;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.147308056121;
        gl_FragColor += texture2D(texture, blurTexCoords[ 8])*0.115876621105;
        gl_FragColor += texture2D(texture, blurTexCoords[ 9])*0.0776744219933;
        gl_FragColor += texture2D(texture, blurTexCoords[10])*0.0443683338718;
        gl_FragColor += texture2D(texture, blurTexCoords[11])*0.0215963866053;
        gl_FragColor += texture2D(texture, blurTexCoords[12])*0.00895781211794;
        gl_FragColor += texture2D(texture, blurTexCoords[13])*0.0044299121055113265;
    }
    else if(amount > 5)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 1])*0.00903788620091297;
        gl_FragColor += texture2D(texture, blurTexCoords[ 2])*0.0217894371884313;
        gl_FragColor += texture2D(texture, blurTexCoords[ 3])*0.0447649434011188;
        gl_FragColor += texture2D(texture, blurTexCoords[ 4])*0.0783687553896337;
        gl_FragColor += texture2D(texture, blurTexCoords[ 5])*0.116912444814051;
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.148624846131007;
        gl_FragColor += texture2D(texture, texcoord         )*0.161003373749691;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.148624846131007;
        gl_FragColor += texture2D(texture, blurTexCoords[ 8])*0.116912444814051;
        gl_FragColor += texture2D(texture, blurTexCoords[ 9])*0.0783687553896337;
        gl_FragColor += texture2D(texture, blurTexCoords[10])*0.0447649434011188;
        gl_FragColor += texture2D(texture, blurTexCoords[11])*0.0217894371884313;
        gl_FragColor += texture2D(texture, blurTexCoords[12])*0.00903788620091297;
    }
    else if(amount > 4)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 2])*0.0221905484924526;
        gl_FragColor += texture2D(texture, blurTexCoords[ 3])*0.0455889997852642;
        gl_FragColor += texture2D(texture, blurTexCoords[ 4])*0.0798114082400501;
        gl_FragColor += texture2D(texture, blurTexCoords[ 5])*0.119064629966432;
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.151360809677289;
        gl_FragColor += texture2D(texture, texcoord         )*0.163967207677023;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.151360809677289;
        gl_FragColor += texture2D(texture, blurTexCoords[ 8])*0.119064629966432;
        gl_FragColor += texture2D(texture, blurTexCoords[ 9])*0.0798114082400501;
        gl_FragColor += texture2D(texture, blurTexCoords[10])*0.0455889997852642;
        gl_FragColor += texture2D(texture, blurTexCoords[11])*0.0221905484924526;
    }
    else if(amount > 3)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 3])*0.0477062557484215;
        gl_FragColor += texture2D(texture, blurTexCoords[ 4])*0.0835180300308369;
        gl_FragColor += texture2D(texture, blurTexCoords[ 5])*0.124594259898761;
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.158390347030314;
        gl_FragColor += texture2D(texture, texcoord         )*0.171582214583331;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.158390347030314;
        gl_FragColor += texture2D(texture, blurTexCoords[ 8])*0.124594259898761;
        gl_FragColor += texture2D(texture, blurTexCoords[ 9])*0.0835180300308369;
        gl_FragColor += texture2D(texture, blurTexCoords[10])*0.0477062557484215;
    }
    else if(amount > 2)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 4])*0.0923272000688803;
        gl_FragColor += texture2D(texture, blurTexCoords[ 5])*0.13773599732728;
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.175096769569969;
        gl_FragColor += texture2D(texture, texcoord         )*0.189680066067741;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.175096769569969;
        gl_FragColor += texture2D(texture, blurTexCoords[ 8])*0.13773599732728;
        gl_FragColor += texture2D(texture, blurTexCoords[ 9])*0.0923272000688803;
    }
    else if(amount > 1)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 5])*0.168929589306119;
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.214751596868314;
        gl_FragColor += texture2D(texture, texcoord         )*0.232637627651133;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.214751596868314;
        gl_FragColor += texture2D(texture, blurTexCoords[ 8])*0.168929589306119;
    }
    else if(amount > 0)
    {
        gl_FragColor += texture2D(texture, blurTexCoords[ 6])*0.324329190908699;
        gl_FragColor += texture2D(texture, texcoord         )*0.351341618182602;
        gl_FragColor += texture2D(texture, blurTexCoords[ 7])*0.324329190908699;
    }
    else gl_FragColor = texture2D(texture, texcoord);
}
