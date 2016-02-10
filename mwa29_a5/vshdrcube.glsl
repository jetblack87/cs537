#version 120

attribute vec4 vPosition, vColor;
varying vec4 color;
uniform mat4 matrix, Projection;
void
main()
{
    gl_Position = Projection * matrix * vPosition;
    color = vColor;
}
