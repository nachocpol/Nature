#version 430 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexcoord;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform sampler2D uHeightMap;
uniform mat4 uModel;
uniform vec2 uChunkPos;
uniform vec4 uClipPlane;

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

out vec2 iTexcoord;
out vec3 iPosition;
out vec3 iColor;
out vec3 iSecondaryColor;

float scale(float fCos)
{
	float x = 1.0 - fCos;
	return uScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void AtmosphericScattering()
{
	vec3 v3LightPos = uSunPosition;
	vec3  v3CameraPos = uCampos;
	v3CameraPos += vec3(0.0f,uInnerRadius,0.0f);
	float fCameraHeight = v3CameraPos.y;

	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 v3Pos = (uModel * vec4(aPosition,1.0f)).xyz;
	v3Pos.y += uInnerRadius;
	vec3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start = v3CameraPos;
	float fDepth = exp((uInnerRadius - fCameraHeight) / uScaleDepth);
	float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
	float fLightAngle = dot(v3LightPos, v3Pos) / length(v3Pos);
	float fCameraScale = scale(fCameraAngle);
	float fLightScale = scale(fLightAngle);
	float fCameraOffset = fDepth*fCameraScale;
	float fTemp = (fLightScale + fCameraScale);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / uSamples;
	float fScaledLength = fSampleLength * uScale;
	vec3 v3SampleRay = v3Ray * fSampleLength;
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample rays
	vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
	vec3 v3Attenuate;
	for(int i=0; i<uSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius - fHeight));
		float fScatter = fDepth*fTemp - fCameraOffset;
		v3Attenuate = exp(-fScatter * (u3InvWavelength * uKr4PI + uKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	iColor = v3FrontColor * (u3InvWavelength * uKrESun + uKmESun);
	iSecondaryColor = v3Attenuate;
}

vec2 GetUv()
{
	float uHeightMapSize = 2048.0f * 14.0f;
	vec2 p = (uModel * vec4(aPosition,1.0f)).xz;
	return p/uHeightMapSize;
}

void main()
{
	AtmosphericScattering();
	vec2 uv = GetUv();
	iTexcoord = uv;
	iPosition = (uModel * vec4(aPosition,1.0f)).xyz;
	gl_ClipDistance[0] = dot(vec4(iPosition,1.0f),uClipPlane);
}