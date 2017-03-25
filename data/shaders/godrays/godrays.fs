#version 430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform sampler2D uColorTexture;
uniform vec2 uSunScreenSpace;
uniform int uSamples;
uniform float uExposure;
uniform float uDecay ;
uniform float uDensity;
uniform float uWeight;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
	//Sun pos in texture coordinates
	vec2 sunPos =  uSunScreenSpace;
	sunPos.x = sin(uTime) + 1.0 * 0.5;
	sunPos.x = uSunScreenSpace.x;
	sunPos.y = 0.5;

	//Extend the rays
 	vec4 finalColor = vec4(0.0);
  	vec2 deltaTc = iTexcoord - sunPos;
  	vec2 curTc = iTexcoord;
  	deltaTc *= (1.0 / float(uSamples)) * uDensity;
 	float illumDecay = 1.0;

  	for(int i=0;i<uSamples;i++)
  	{
    	curTc -= deltaTc;
	    vec4 color = texture(uColorTexture,curTc);
	    color *= illumDecay * uWeight;

	    finalColor += color;
	    illumDecay *= uDecay;
  	}
  	oColor = finalColor * uExposure;
}