#version 430 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexcoord;

uniform mat4 uModel;
uniform vec2 uChunkPos;
uniform vec4 uClipPlane;

out vec2 iTexcoord;
out vec3 iPosition;

vec2 GetUv()
{
	float uHeightMapSize = 2048.0f;
	vec2 p = (uModel * vec4(aPosition,1.0f)).xz;
	return p/uHeightMapSize;
}

void main()
{
	vec2 uv = GetUv();
	iTexcoord = uv;
	iPosition = (uModel * vec4(aPosition,1.0f)).xyz;
	gl_ClipDistance[0] = dot(vec4(iPosition,1.0f),uClipPlane);
}