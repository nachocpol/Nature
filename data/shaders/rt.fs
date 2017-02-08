#version 430 core

uniform sampler2D uColorTexture;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
	oColor = vec4(iTexcoord.x,iTexcoord.y,0.0f,1.0f);
}