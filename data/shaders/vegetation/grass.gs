#version 430 core

layout (points) in;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};
in float[] gLogz;
in vec3[] gPosition;

layout (triangle_strip,max_vertices = 3) out;
out float iLogz;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	iLogz = gLogz[0];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position ;
	iLogz = gLogz[0];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position;
	iLogz = gLogz[0];
	EmitVertex();
	EndPrimitive();
}