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

out vec4 oColor;

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

    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}