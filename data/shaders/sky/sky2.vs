#version 430 core

layout (location = 0)in vec3 aPosition;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform mat4 uModel;

out vec3 iPosition;

void main()
{
	// Draw as unit sphere
	mat3 v = mat3(uView);
	vec3 displacedPos = aPosition * 2.0f;	// radius = 1
	gl_Position = uProjection * mat4(v) * vec4(displacedPos,1.0f);

	iPosition = displacedPos;
}