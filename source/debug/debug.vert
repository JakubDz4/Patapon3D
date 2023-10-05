#version 330 core

layout(location = 0) in vec4 aPos;

layout(std140) uniform uMatrices
{
    mat4 view;
	mat4 proj;
};

void main()
{
    gl_Position = proj * view * aPos;
}