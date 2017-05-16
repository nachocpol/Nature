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
};

uniform mat4 uModel;

out vec2 iTexcoord;
out vec3 iCPos;
out float iLogz;

void main()
{
	iTexcoord = aTexcoord;
	gl_Position = uProjection * uView * uModel *  vec4(aPosition,1.0f);
	iCPos = gl_Position.xyz;

	float Fcoef = 2.0 / log2(uCamfar + 1.0);
	iLogz = 1.0f + gl_Position.w;
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
}	