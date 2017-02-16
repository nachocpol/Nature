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

out vec3 v3Direction;
out vec3 c0;
out vec3 c1;
out vec3 iSunDir;
float scale(float fCos)
{
	float fScaleDepth = 0.25f;		// The scale depth (i.e. the altitude at which the atmosphere's average density is found)
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
}

void main()
{
	float Kr = 0.0025f;	 // Rayleigh scattering constant
	float ESun = 20.0f; // Sun brightness constant
	float Km = 0.0010f;		// Mie scattering constant
	vec3 wavelength = vec3(0.650f,0.570f,0.475f);
	vec3 pow4WaveLength = pow(wavelength,vec3(4.0f));

	const float PI = 3.141517f;
	const int nSamples = 4;
	const float fSamples = 4.0;

	vec3  v3CameraPos = vec3(0.0f,0.956f,0.0f);		// The camera's current position
	//float sunY = sin(uTime * 0.5f);
	vec3  v3LightPos = vec3(0.0f,0.0f,1.0f);		// The direction vector to the light source
	iSunDir = v3LightPos;
	vec3  v3InvWavelength = 1.0f / pow4WaveLength;	// 1 / pow(wavelength, 4) for the red, green, and blue channels
	float fCameraHeight = v3CameraPos.y;	// The camera's current height
	float fCameraHeight2 = pow(fCameraHeight,2.0f);	// fCameraHeight^2
	float fOuterRadius = 1.0f;		// The outer (atmosphere) radius
	float fOuterRadius2 = pow(fOuterRadius,2.0f);	// fOuterRadius^2
	float fInnerRadius = 0.95f;		// The inner (planetary) radius
	float fInnerRadius2 = pow(fInnerRadius,2.0f);	// fInnerRadius^2
	float fKrESun = Kr * ESun;			// Kr * ESun
	float fKmESun = Km * ESun;			// Km * ESun
	float fKr4PI = Kr * 4.0f * PI;			// Kr * 4 * PI
	float fKm4PI = Km * 4.0f * PI;			// Km * 4 * PI
	float fScale = 1.0f / (fOuterRadius - fInnerRadius);			// 1 / (fOuterRadius - fInnerRadius)
	float fScaleDepth = 0.1f;		// The scale depth (i.e. the altitude at which the atmosphere's average density is found)
	float fScaleOverScaleDepth = fScale/fScaleDepth;	// fScale / fScaleDepth

	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 v3Pos = (uModel * vec4(aPosition,0.0f)).xyz;
	vec3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start = v3CameraPos;
	float fHeight = length(v3Start);
	float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth*scale(fStartAngle);

	// Initialize the scattering loop variables
	//gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0);
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	vec3 v3SampleRay = v3Ray * fSampleLength;
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample rays
	vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
	for(int i=0; i<nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
		vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	c1 = v3FrontColor * fKmESun;
	c0 = v3FrontColor * (v3InvWavelength * fKrESun);
	v3Direction = v3CameraPos - v3Pos;

	mat3 v = mat3(uView);
	vec3 displacedPos = aPosition;
	displacedPos.y -= 0.2f;
	gl_Position = uProjection * mat4(v)  *  vec4(displacedPos,1.0f);
}	