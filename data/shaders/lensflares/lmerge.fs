#version 430 core

uniform sampler2D uColorTexture;
uniform sampler2D uLens;
uniform sampler2D uLensDust;
uniform sampler2D uLensStar;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
	vec4 scene = texture(uColorTexture,iTexcoord);
	vec4 lens = texture(uLens,iTexcoord);
	vec4 lensDust = texture(uLensDust,iTexcoord);
	//lensDust += texture(uLensStar,iTexcoord);
	oColor = scene + (lens * lensDust);
}