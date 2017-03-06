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

uniform float uG2;
uniform float uG;

in vec3 v3Direction;
in vec3 iSunDir;
in vec3 iColor;
in vec3 iSecondaryColor;

out vec4 oColor;

void main()
{
	//oColor = vec4(iColor,1.0f)	;
	float fCos = dot(normalize(iSunDir), v3Direction) / length(v3Direction);
	float fMiePhase = 1.5 * ((1.0 - uG2) / (2.0 + uG2)) * (1.0 + fCos*fCos) / pow(1.0 + uG2 - 2.0*uG*fCos, 1.5);
	oColor.xyz = (iColor + fMiePhase * iSecondaryColor);
	oColor.a = oColor.b;
}