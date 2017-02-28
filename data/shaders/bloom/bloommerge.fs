#version 430 core

uniform sampler2D uColorTexture;
uniform sampler2D uBloomTexture;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
	//oColor = texture(uBloomTexture,iTexcoord);
	//oColor = texture(uColorTexture,iTexcoord);
  	oColor = texture(uColorTexture,iTexcoord) + texture(uBloomTexture,iTexcoord);
}