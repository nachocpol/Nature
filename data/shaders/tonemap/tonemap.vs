#version 430 core

layout (location = 0)in vec3 aPosition;
layout (location = 1)in vec2 aTexcoord;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
};

out vec2 iTexcoord;

void main()
{
	iTexcoord = aTexcoord;
	gl_Position = vec4(aPosition,1.0f);
}