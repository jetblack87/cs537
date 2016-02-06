#version 120

attribute vec4 vPosition, vColor;
varying vec4 color;
uniform mat4 matrix;

void
main()
{
    gl_Position = matrix * vPosition;
    color = vColor;
}
