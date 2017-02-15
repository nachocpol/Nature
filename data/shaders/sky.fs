#version  430 core

in vec3 v3Direction;
in vec3 iMainColor;
in vec3 iSecondaryColor;

out vec4 oColor;

void main()
{
	vec3  v3LightPos = vec3(0.5f,1.0f,0.0f);
	float g = -0.990f;
    float g2 = pow(g,2.0f);

	float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
	
	oColor.xyz = iMainColor + fMiePhase * iSecondaryColor;
	oColor.a = 1.0f;
}