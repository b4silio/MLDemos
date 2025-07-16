#version 120

// the data we get from the scene
attribute vec4 vertex;
attribute vec4 vertexNormal;
attribute vec3 vertexColor;
attribute vec4 barycentric;

// the model+projection matrix and viewport information
struct lightData
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};
uniform lightData lights[3];

// the uniforms
uniform mat4 mvMatrix; // model + view matrix
uniform mat4 mvpMatrix; // model + view + projection matrix
uniform mat3 normalMatrix;
uniform vec4 materialColor; // base material color (multiplied to vertex colors)
uniform bool transparency;

// parameters related to vertex/fragment positions
varying vec3 normal;
varying vec4 vertPosition;
varying vec4 vertProjected;
varying vec4 vBC;
varying vec3 vertexSource;

// parameters related to material/lights
varying float alpha;
varying vec4 ambientColor;

const int MAX_LIGHTS = 3;
varying vec3 lightPositions[MAX_LIGHTS];
varying vec4 diffuseColors[MAX_LIGHTS];

void main()
{
    vertexSource = vertex.xyz;
    gl_Position = mvpMatrix * vertex;
    vertProjected = gl_Position;
    vertPosition = mvMatrix * vertex;
    normal = normalMatrix * vertexNormal.xyz;
    vBC = barycentric;

    vec4 color = materialColor*vec4(vertexColor,1);
    ambientColor = vec4(0,0,0,0);
    if(!transparency) color.a = 1.f;
    alpha = color.a;

    for(int i=0;i<MAX_LIGHTS;i++)
    {
        lightPositions[i] = lights[i].position.xyz;
        ambientColor += color * lights[i].ambient;
        diffuseColors[i] = color * lights[i].diffuse;
    }
}
