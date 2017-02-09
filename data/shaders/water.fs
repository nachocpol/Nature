#version  430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
};

uniform sampler2D uReflectionTexture;
uniform sampler2D uRefractionTexture;

in vec2 iTexcoord;
in vec4 iCPos;
in vec3 iWPos;

out vec4 oColor;

float CalcFresnel(vec3 normal)
{
	vec3 toView = normalize(uCampos - iWPos);
	float f = max(dot(toView,normal),0.0);
	return f;
}

vec3 GetWaterColor()
{
	vec2 uv = iCPos.xy / iCPos.w;
	uv = uv * 0.5f + 0.5f;

	vec3 reflectCol = texture(uReflectionTexture,vec2(uv.x,-uv.y)).xyz;
	vec3 refractCol = texture(uRefractionTexture,vec2(uv.x,-uv.y)).xyz;

	return mix(reflectCol,refractCol,CalcFresnel(vec3(0,1,0)));
}

void main()
{
	vec2 uv = iCPos.xy / iCPos.w;
	uv = uv * 0.5f + 0.5f;

	vec3 reflectCol = texture(uReflectionTexture,vec2(uv.x,-uv.y)).xyz;
	vec3 refractCol = texture(uRefractionTexture,uv).xyz;

	vec3 c =  mix(reflectCol,refractCol,CalcFresnel(vec3(0,1,0)));
	oColor = vec4(c,1.0f);
	//oColor = vec4(GetWaterColor(),1.0f);
}