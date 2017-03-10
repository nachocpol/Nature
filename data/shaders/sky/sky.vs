#version 430 core

layout (location = 0)in vec3 aPosition;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform mat4 uModel;

// Atmospheric scattering parameters
uniform vec3 uSunPosition;
uniform int uSamples;           
uniform float uFSamples;
uniform float uInnerRadius;
uniform vec3  uPow4WaveLength;
uniform vec3  u3InvWavelength;	
uniform float uOuterRadius2;	
uniform float uInnerRadius2;	
uniform float uKrESun;			
uniform float uKmESun;			
uniform float uKr4PI;			
uniform float uKm4PI;			
uniform float uScale;			
uniform float uScaleDepth;	
uniform float uScaleOverScaleDepth;	

out vec3 v3Direction;
out vec3 iSunDir;
out vec3 iColor;
out vec3 iSecondaryColor;

float scale(float fCos)
{
	float x = 1.0 - fCos;
	return uScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
}

void main()
{
	vec3  v3CameraPos = uCampos;
	v3CameraPos += vec3(0.0f,uInnerRadius,0.0f);
	float fCameraHeight = v3CameraPos.y;
	vec3  v3LightPos = uSunPosition;		// The direction vector to the light source
	iSunDir = v3LightPos;

	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 v3Pos = (uModel * vec4(aPosition,1.0f)).xyz;
	vec3 v3Ray = (v3Pos - v3CameraPos);
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start = v3CameraPos;
	float fHeight = length(v3Start);
	float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius - fCameraHeight));
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth*scale(fStartAngle);

	// Initialize the scattering loop variables
	//gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0);
	float fSampleLength = fFar / uFSamples;
	float fScaledLength = fSampleLength * uScale;
	vec3 v3SampleRay = v3Ray * fSampleLength;
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample rays
	vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
	for(int i=0; i<uSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius - fHeight));
		float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
		vec3 v3Attenuate = exp(-fScatter * (u3InvWavelength * uKr4PI + uKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	iSecondaryColor = v3FrontColor * uKmESun;
	iColor = v3FrontColor * (u3InvWavelength * uKrESun);
	v3Direction = v3CameraPos - v3Pos;
	
	// Draw as skybox, on top of everything
	mat3 v = mat3(uView);
	vec3 displacedPos = aPosition * 100.0f;
	displacedPos.y -= 48.8f;
	gl_Position = uProjection * mat4(v) * vec4(displacedPos,1.0f);
}	
