#version 120

// the data we get from the scene
attribute vec4 vertex;
attribute vec3 color;

// the model+projection matrix and viewport information
uniform mat4 matrix;
uniform vec4 viewport;

// the stuff we want to pass to the fragment shader
varying vec2 screenCoord;
varying float sampleR;
varying float sampleG;
varying float sampleB;

void main(void)
{
    gl_Position = matrix * vertex;
    sampleR = color.r;
    sampleG = color.g;
    sampleB = color.b;
    screenCoord = gl_Position.xy / gl_Position.w;
    screenCoord.x = (screenCoord.x+1.)*viewport[2]*0.5 + viewport[0]; // viewport transformation
	screenCoord.y = (screenCoord.y+1.)*viewport[3]*0.5 + viewport[1]; // viewport transformation
}
