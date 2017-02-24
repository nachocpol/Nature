#version 430 core

layout (location = 0)in vec3 aPosition;
layout (location = 1)in vec2 aTexcoord;
layout (location = 6) in mat4 aInstancedModel;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

out vec2 iTexcoord;
out float iLogz;

void main()
{
	iTexcoord = aTexcoord;
	vec3 newPos = aPosition;
	newPos.z += (sin(uTime * 0.8f) * 0.25f) * aPosition.y;

	gl_Position = uProjection * uView * aInstancedModel *  vec4(newPos,1.0f);
	
	float Fcoef = 2.0 / log2(uCamfar + 1.0);
	iLogz = 1.0f + gl_Position.w;
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
}	