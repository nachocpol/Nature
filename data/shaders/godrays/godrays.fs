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
uniform vec3 uSunScreenSpace;
uniform vec3 uSunDirection;
uniform int uSamples;
uniform float uExposure;
uniform float uDecay ;
uniform float uDensity;
uniform float uWeight;

in vec2 iTexcoord;

out vec4 oColor;

void main()
{	
	// Early exit if its night time (dont cast god rays)
	if(uSunDirection.y <= 0.0 || uSunScreenSpace.z <= 0.0)
	{
		oColor = texture(uColorTexture,iTexcoord);
		return;		
	}

	vec2 sunPos =  clamp(uSunScreenSpace.xy,vec2(0.0),vec2(1.0));

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