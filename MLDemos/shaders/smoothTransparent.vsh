// the data we get from the scene
attribute vec4 vertex;
attribute vec4 vertexNormal;

// the model+projection matrix and viewport information
uniform mat4 matrix;
uniform mat4 normalMatrix;
uniform vec4 viewport;

varying vec3 normal, lightDir;
varying vec4 diffuse;


void main()
{
    gl_Position = matrix * vertex;

    vec3 transformedNormal = normalMatrix * vertexNormal;

    normal = normalMatrix * vertexNormal;

    if(vec3(gl_LightSource[0].position) != vec3(0.0, 0.0, 0.0))
    {
        lightDir = vec3(gl_LightSource[0].position);
        diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    }
}
