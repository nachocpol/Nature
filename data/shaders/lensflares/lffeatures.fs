#version 430 core

uniform sampler2D uColorTexture;
uniform int uGhostSamples;
uniform float uGhostDispers;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
	vec2 uv = -iTexcoord + vec2(1.0f);
	vec2 texelsize = 1.0f / vec2(textureSize(uColorTexture,0));

	// Vector to image center
	vec2 ghostVec = fract(vec2(0.5f) - uv) * uGhostDispers;

	// Sample ghosts
	vec4 res = vec4(0.0f);
	for(int i=0;i<uGhostSamples;i++)
	{
		vec2 off = (uv + ghostVec * float(i));
		res += texture(uColorTexture,off);
	}
	oColor = res;
}