#version 430 core

uniform sampler2D uColorTexture;
uniform vec2 uSunScreenSpace;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{
	float uExposure = 0.5;
    float uDecay = 0.95;
    float uDensity = 3.5;
    float uWeight = 0.97;
    const int uSamples = 190;

	//Sun pos in texture coordinates
	vec2 sunPos =  uSunScreenSpace * 0.5 + 0.5;
	sunPos = clamp(sunPos,vec2(0.0),vec2(1.0));

	//Extend the rays
 	vec4 finalColor = vec4(0.0);
  	vec2 deltaTc = iTexcoord - sunPos;
  	vec2 curTc = iTexcoord;
  	deltaTc *= 1.0 / float(uSamples) * uDensity;
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