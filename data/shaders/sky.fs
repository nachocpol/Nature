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

in vec3 v3Direction;
in vec3 c0;
in vec3 c1;
in vec3 iSunDir;

out vec4 oColor;

// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2)
{
	return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
}

// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2)
{
	return 0.75 + 0.75 * fCos2;
}

void main (void)
{
	vec3  v3LightPos = iSunDir;
	
	const float g = -0.990f;
	const float g2 = g * g;

	float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
	float fCos2 = fCos * fCos;

	vec3 color =	getRayleighPhase(fCos2) * c0 +
					getMiePhase(fCos, fCos2, g, g2) * c1;

 	oColor = vec4(color, 1.0);
	//oColor.a = oColor.b;
}