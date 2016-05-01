#version 150

in vec4 vertex;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main()
{
    gl_Position = projMatrix * viewMatrix * vertex;
}
