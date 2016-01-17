#version 120

attribute vec3 vPosition, vColor;
varying vec3 color;

void
main()
{
    gl_Position = vec4(vPosition, 1);
    color = vColor;
}
