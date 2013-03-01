#version 120

// the data we get from the scene
attribute vec3 vertex;

// the model+projection matrix and viewport information
uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;

// the stuff we want to pass to the fragment shader
varying float asd; // for god knows what reason, if I put this here the next points will not be interpolated
varying float vX;
varying float vY;
varying float vZ;
varying float vW;

void main(void)
{
    gl_Position = mvpMatrix * vec4(vertex,1);
    vec4 lightVertex = mvpMatrix * vec4(vertex,1);
//    vec4 lightVertex = vec4(vertex,1);
    vX = lightVertex.x;
    vY = lightVertex.y;
    vZ = lightVertex.z;
    vW = lightVertex.w;
    asd = 1;
}
