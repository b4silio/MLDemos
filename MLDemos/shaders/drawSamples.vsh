#version 120

// the data we get from the scene
attribute vec4 vertex;
attribute vec4 color;

// the model+projection matrix and viewport information
uniform mat4 matrix;

// the stuff we want to pass to the fragment shader
varying float sampleR;
varying float sampleG;
varying float sampleB;
varying float stuff;

void main(void)
{
    stuff = 1.;
    sampleR = color.r;
    sampleG = color.g;
    sampleB = color.b;
    gl_Position = matrix * vertex;
}
