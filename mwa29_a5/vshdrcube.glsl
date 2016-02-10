#version 120

attribute vec4 vPosition, vColor;
varying vec4 color;
uniform mat4 ModelView;

void
main()
{
    gl_Position = ModelView * vPosition;
    color = vColor;
}
