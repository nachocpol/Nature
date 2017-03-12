#version  430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

in vec2 iTexcoord;
in float iLogz;
flat in int iLod;
in vec3 iPosition;
in vec3 iWPos;

out vec4 oColor;

vec3 GetFog(   in vec3  rgb,        // original color of the pixel
               in float distance,   // camera to point distance
               in vec3  rayOri,     // camera position
               in vec3  rayDir )    // camera to point vector
{
    float c = 0.01f;
    float b = 0.01f;
    float fogAmount = c * exp(-rayOri.y*b) * (1.0-exp( -distance*rayDir.y*b ))/rayDir.y;
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}

void main()
{	
	
	if(iLod == 1)
	{
		oColor = vec4(0.0f,1.0f,1.0f,1.0f);
	}
	else if(iLod == 2)
	{
		oColor = vec4(0.0f,0.0f,1.0f,1.0f);
	}
	else if(iLod == 3)
	{
		oColor = vec4(0.0f,1.0f,0.0f,1.0f);
	}
	else if(iLod == 4)
	{
		oColor = vec4(1.0f,0.0f,0.0f,1.0f);
	}
	
	/*
	float wShade = iPosition.x * sign(iPosition.x);
	wShade *= 10.0f;	// so the half width its 1
	wShade = pow(wShade,0.6f);
	oColor = vec4(0.07f,0.38f,0.1f,1.0f) * pow(iPosition.y,2.0f) * wShade;
	oColor.xyz = GetFog(oColor.xyz,distance(iWPos,uCampos),uCampos,normalize(iWPos - uCampos));
	*/
	
	// Log z buffer
    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}