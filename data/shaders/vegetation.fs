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

uniform sampler2D uAlbedoTexture;
uniform sampler2D uOpacityTexture;

in vec2 iTexcoord;
in float iLogz;

out vec4 oColor;

void main()
{
	vec3 albedo = texture(uAlbedoTexture,iTexcoord).xyz;
	float alpha = texture(uOpacityTexture,iTexcoord).x;
	oColor = vec4(albedo,alpha);
	//oColor = vec4(iTexcoord.xy,0.0f,1.0f);
    
    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}