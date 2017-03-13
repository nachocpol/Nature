#version 430 core

layout (location = 0)in vec2 aPosition;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

out vec2 gPosition;

void main()
{
	gPosition = aPosition;
}	