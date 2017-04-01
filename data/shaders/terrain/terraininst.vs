#version 430 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexcoord;
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

uniform sampler2D uHeightMap;
uniform vec2 uChunkPos;
uniform vec4 uClipPlane;
uniform float uTerrainHeightScale;
uniform float uTerrainScale;

out vec2 iTexcoord;
out vec3 iPosition;
out float iLogz;


vec2 GetUv()
{
	// texture size!
	float uHeightMapSize = 2048.0f * uTerrainScale;
	vec2 p = (aInstancedModel * vec4(aPosition,1.0f)).xz;
	return clamp(p/uHeightMapSize,vec2(0.001),vec2(0.999));
}

float GetHeight(vec2 uv)
{ 
    return texture(uHeightMap,uv).x * uTerrainHeightScale;
}

void main()
{
	vec2 uv = GetUv();
	iTexcoord = uv;
	vec3 curPos = aPosition;
	curPos.y = GetHeight(uv);
	iPosition = (aInstancedModel * vec4(curPos,1.0f)).xyz;
	gl_ClipDistance[0] = dot(vec4(iPosition,1.0f),uClipPlane);
	gl_Position = uProjection * uView * vec4(iPosition, 1.0f);

	float Fcoef = 2.0 / log2(uCamfar + 1.0);
    iLogz = 1.0f + gl_Position.w;
    gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
}