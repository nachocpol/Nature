#version 430 core

uniform sampler2D uColorTexture;
uniform vec4 uScale;
uniform vec4 uBias;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
  	vec4 cTexture = texture(uColorTexture,iTexcoord);
	oColor = max(vec4(0.0f),cTexture + uBias) * uScale;
}