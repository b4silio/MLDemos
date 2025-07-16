#version 120

uniform vec4 surfaceStyle; // surface style: x=face, y=wire, z=blurry, w=nothing

varying vec3 normal;
varying vec4 vertPosition;
varying vec4 vertProjected;
varying vec4 vBC;
varying vec3 vertexSource;

// the model+projection matrix and viewport information
struct lightData
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};
uniform lightData lights[3];
uniform float shininess;
uniform float specularity;

varying float alpha;
varying vec4 ambientColor;
const int MAX_LIGHTS = 3;
varying vec3 lightPositions[MAX_LIGHTS];
varying vec4 diffuseColors[MAX_LIGHTS];
varying vec4 speculars[MAX_LIGHTS];

void main()
{
    float NdotL, iNdotL;
    vec3 N, L, E, H, R, iR;
    float Spec, theDot, iSpec, iDot;

    /* a fragment shader can't write a verying variable, hence we need
        a new variable to store the normalized interpolated normal */
    N = normalize(normal);

    vec4 color = ambientColor;
    vec4 specColor = vec4(0,0,0,0);
    //for(int i=0;i<1;i++)

    for(int i=0;i<MAX_LIGHTS;i++)
    {
        // Compute the light direction
        L = normalize(lightPositions[i] - vertPosition.xyz);

        // compute the dot product between normal and ldir
        NdotL = dot(N,L);

        if (NdotL > 0.0)
        {
            color += (diffuseColors[i] * NdotL);
        }

        // and now the specularity
        // http://www.lighthouse3d.com/tutorials/glsl-tutorial/directional-lights-ii/
        L = normalize(lightPositions[i] - vertProjected.xyz);
        E = normalize(-vertProjected.xyz);
        // Phong
        R = reflect(-L, N);
        theDot = max(dot(R,E),0.0);
        // Blinn-Phong
        //H = E - L;
        //theDot = max(dot(n,H),0.0);

        Spec = pow(theDot,shininess)*specularity;
        specColor += lights[i].specular*Spec;
    }
//    specColor.a = 1.0;
    color.a = alpha;

    // we draw the solid surface (or not)
    if(surfaceStyle.x > 0.1) gl_FragColor = color + specColor;
    else gl_FragColor = vec4(1,1,1,0);

    // we draw the wireframe (or not)
    if(surfaceStyle.y > 4.1) // isolines on all axes
    {
        float z = abs(vertexSource.z);
        float iZ = float(int(z * 100.f)/100.f);
        z = z - iZ;
        if(z < 0.001)
        {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        float y = abs(vertexSource.y);
        float iY = float(int(y * 100.f)/100.f);
        y = y - iY;
        if(y < 0.001)
        {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        float x = abs(vertexSource.x);
        float iX = float(int(x * 100.f)/100.f);
        x = x - iX;
        if(x < 0.001)
        {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
    }
    else if(surfaceStyle.y > 3.1) // isolines on z
    {
        float z = abs(vertexSource.z);
        float iZ = float(int(z * 100.f)/100.f);
        z = z - iZ;
        if(z < 0.001) gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else if(surfaceStyle.y > 2.1) // isolines on y
    {
        float y = abs(vertexSource.y);
        float iY = float(int(y * 100.f)/100.f);
        y = y - iY;
        if(y < 0.001) gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else if(surfaceStyle.y > 1.1) // isolines on x
    {
        float x = abs(vertexSource.x);
        float iX = float(int(x * 100.f)/100.f);
        x = x - iX;
        if(x < 0.001) gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else if(surfaceStyle.y > 0.1) // standard wireframe
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
