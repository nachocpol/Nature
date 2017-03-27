#version 430 core

uniform sampler2D uColorTexture;
uniform sampler2D uGodRaysTexture;

in vec2 iTexcoord;

out vec4 oColor;

const float gamma = 2.2f;

vec3 ReinhardToneMapping(vec3 color)
{
	float exposure = 1.5;
	color *= exposure/(1. + color / exposure);
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 LumaBasedReinhardToneMapping(vec3 color)
{
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 Uncharted2ToneMapping(vec3 color)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	float W = 11.2f;
	float exposure = 2.0f;
	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
	color = pow(color, vec3(1.0f / gamma));
	return color;
}

vec3 Vignette(vec3 input,vec2 uv)
{
	float d = distance(uv,vec2(0.5));
	return input * smoothstep(0.85f,0.6f,d);
}

void main()
{
	vec3 scene = texture(uColorTexture,iTexcoord).xyz;
	/*
	vec3 godRays = texture(uGodRaysTexture,iTexcoord).xyz;
	scene += godRays;
	*/
	vec3 corrected = ReinhardToneMapping(scene);
	corrected = Vignette(corrected,iTexcoord);
	oColor = vec4(corrected,1.0f);
}