#version 430 core

layout (location = 0)in vec3 aPosition;
layout (location = 1)in vec2 aTexcoord;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
	float uAspect;
};

out vec2 iTexcoord;
out vec3 iPos;

void main()
{
	iTexcoord = aTexcoord;
	iPos = vec3(aPosition.x,aPosition.y,-1.0f);
	gl_Position = vec4(aPosition,1.0f);
}