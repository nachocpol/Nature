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
	// EleSize - 1 !!!
	vec2 cPosStart = vec2(uChunkPos.x * 63.0f , uChunkPos.y * 63.0f);
	vec2 curUv = vec2((aPosition.xz + cPosStart) / 1024.0f);
	return curUv;
}

void main()
{
	vec2 uv = GetUv();
	iTexcoord = uv;
	iPosition = (uModel * vec4(aPosition,1.0f)).xyz;
	/*
	vec4 wPos = uModel * vec4(aPosition,1.0f);
	wPos.y = texture(uHeightMap,uv).x * 100.0f;
	iPosition = wPos.xyz;
	gl_Position = uProjection * uView * wPos;
	gl_ClipDistance[0] = dot(wPos,uClipPlane);
	*/
}