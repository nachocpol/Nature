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

out float gLogz;

void main()
{
	gl_Position = uProjection * uView * vec4(aPosition,1.0);
	
	float Fcoef = 2.0 / log2(uCamfar + 1.0);
	gLogz = 1.0f + gl_Position.w;
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
}	