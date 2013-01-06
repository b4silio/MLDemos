#version 120

varying vec3 normal;
varying vec4 vertPosition;
varying vec4 vBC;
varying float vertexY;

varying float alpha;
varying float shininess;
varying vec4 ambientColor;
varying vec4 style;
const int MAX_LIGHTS = 3;
varying vec3 lightPositions[MAX_LIGHTS];
varying vec4 diffuseColors[MAX_LIGHTS];
varying vec4 speculars[MAX_LIGHTS];



void main()
{
    vec3 n,lightDir;
    float NdotL;
    vec3 L, Eye, H, R;
    float Spec, theDot;

    /* a fragment shader can't write a verying variable, hence we need
        a new variable to store the normalized interpolated normal */
    n = normalize(normal);

    vec4 color = ambientColor;
    vec4 specColor = vec4(0,0,0,0);
    //for(int i=0;i<1;i++)
    for(int i=0;i<MAX_LIGHTS;i++)
    {
        // Compute the light direction
        lightDir = normalize(lightPositions[i] - vertPosition.xyz);

        // compute the dot product between normal and ldir
        NdotL = max(dot(n,lightDir),0.0);

        if (NdotL > 0.0)
        {
            color += (diffuseColors[i] * NdotL);

            // http://www.lighthouse3d.com/tutorials/glsl-tutorial/directional-lights-ii/

            L = normalize(lightDir);
            Eye = normalize(-vertPosition.xyz);

            // Phong
            R = 2*n*dot(n,L) - L;
            theDot = max(dot(R,Eye),0.0);

            // Blinn-Phong
            //H = Eye - L;
            //theDot = max(dot(n,H),0.0);

            Spec = pow(theDot,shininess);
            specColor += speculars[i] * Spec;
        }
    }
    color.a = alpha;

    // we draw the solid surface (or not)
    if(style.x > 0.1) gl_FragColor = color + specColor;
    else gl_FragColor = vec4(1,1,1,0);

    // we draw the wireframe (or not)
    if(style.y > 1.1) // isolines on y
    {
        float y = abs(vertexY);
        while (y > 1) y-= 1;
        while (y > 0.1) y-= 0.1;
        while (y > 0.01) y-= 0.01;
        if(y < 0.001)
        {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    else if(style.y > 0.1) // standard wireframe
    {
        if(any(lessThan(vBC, vec4(0.04))))
        {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }

        /*
        vec3 d = fwidth(vBC.xyz);
        vec3 a3 = smoothstep(vec3(0.0), d*3, vBC.xyz);// we want a 3 pixel wide line
        float edgeFactor = min(min(a3.x, a3.y), a3.z);
        //gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        gl_FragColor += vec4(0.0, 0.0, 0.0, (1.0-edgeFactor)*0.95);
        */
    }
}
