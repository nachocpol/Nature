#version 430 core

uniform sampler2D uColorTexture;
uniform float uIntensity;
uniform bool uHorizontal;

in vec2 iTexcoord;

out vec4 oColor;

float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

vec3 GetColorTexture(vec2 uv,float intensity)
{
	return texture(uColorTexture,uv).xyz * intensity;
}

void main()
{
  	vec2 texelsize = 1.0f / textureSize(uColorTexture, 0);
    vec3 result = texture(uColorTexture, iTexcoord).rgb * uIntensity * weight[0];
    if(uHorizontal)
    {
	    for(int i = 1; i < 5; ++i)
	    {
	        result += GetColorTexture(iTexcoord + vec2(texelsize.x * i, 0.0f),uIntensity) * weight[i];
	        result += GetColorTexture(iTexcoord - vec2(texelsize.x * i, 0.0f),uIntensity) * weight[i];
	    }
    }
    else
    {
	    for(int i = 1; i < 5; ++i)
	    {
	        result += GetColorTexture(iTexcoord + vec2(0.0f,texelsize.y * i),1.0f) * weight[i];
	        result += GetColorTexture(iTexcoord - vec2(0.0f,texelsize.y * i),1.0f) * weight[i];
	    }
    }
    oColor = vec4(result, 1.0);
}