#version 120

// the data we get from the scene
attribute vec4 vertex;
attribute vec4 color;

// the model+projection matrix and viewport information
uniform mat4 matrix;
uniform mat4 lightMvpMatrix;
uniform float pointSize;

// this is to reproject the values of the vertex into shadowMap coordinates (0-1)
const mat4 ScaleMatrix = mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

// the stuff we want to pass to the fragment shader
varying float asd;
varying float sampleR;
varying float sampleG;
varying float sampleB;
varying float stuff;
varying float lvX;
varying float lvY;
varying float lvZ;
varying float lvW;

void main(void)
{
    stuff = 1.;
    sampleR = color.r;
    sampleG = color.g;
    sampleB = color.b;
    gl_Position = matrix * vertex;
    //vec4 lightVertex = ScaleMatrix * lightMvpMatrix * vertex;
    vec4 lightVertex = vertex;
    lvX = lightVertex.x;
    lvY = lightVertex.y;
    lvZ = lightVertex.z;
    lvW = lightVertex.w;
//    asd = 1.;
}
