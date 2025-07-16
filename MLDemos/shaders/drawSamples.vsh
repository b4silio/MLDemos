#version 120
#extension GL_EXT_gpu_shader4 : require

// the data we get from the scene
attribute vec4 vertex;
attribute vec4 color;

// the model+projection matrix and viewport information
uniform mat4 matrix;

// the stuff we want to pass to the fragment shader
flat varying vec4 c;

void main(void)
{
    c = color;
    gl_Position = matrix * vertex;
}
