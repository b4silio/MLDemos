#version 120

// the data we get from the scene
attribute vec4 vertex;
attribute vec4 vertexNormal;
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
uniform vec4 viewport;
uniform vec4 materialColor;
uniform vec4 surfaceStyle; // surface style: 0=face, 1=wire, 2=blurry, 3=nothing

// parameters related to vertex/fragment positions
varying vec3 normal;
varying vec4 vertPosition;
varying vec4 vBC;
varying float vertexY;

// parameters related to material/lights
varying float alpha;
varying vec4 ambientColor;
varying float shininess;
varying vec4 style;
const int MAX_LIGHTS = 3;
varying vec3 lightPositions[MAX_LIGHTS];
varying vec4 diffuseColors[MAX_LIGHTS];
varying vec4 speculars[MAX_LIGHTS];

void main()
{
    vertexY = vertex.y;
    gl_Position = mvpMatrix * vertex;
    vertPosition = mvMatrix * vertex;
    normal = normalMatrix * vertexNormal.xyz;
    vBC = barycentric;
    style = surfaceStyle;

    ambientColor = vec4(0,0,0,0);
    shininess = 8.0;
    alpha = materialColor.a;

    for(int i=0;i<MAX_LIGHTS;i++)
    {
        ambientColor += materialColor * lights[i].ambient;
        lightPositions[i] = lights[i].position.xyz;
        diffuseColors[i] = materialColor * lights[i].diffuse;
        speculars[i] = lights[i].specular; // speculars are white
        //speculars[i] = materialColor * lights[i].specular; // speculars are material-colored
    }
}
