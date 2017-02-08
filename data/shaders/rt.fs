#version 430 core

uniform sampler2D uColorTexture;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
  vec3 cTexture = texture(uColorTexture,iTexcoord).xyz;
	oColor = vec4(cTexture,1.0f);
}