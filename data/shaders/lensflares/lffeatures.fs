#version 430 core

uniform sampler2D uColorTexture;
uniform int uGhostSamples;
uniform float uGhostDispers;
uniform float uHaloWidth;
uniform float uDistortion;

in vec2 iTexcoord;

out vec4 oColor;

vec3 TextureDistorted(sampler2D tex,vec2 texcoord,vec2 direction, vec3 distortion )
{
	return vec3
	(
	 texture(tex, texcoord + direction * distortion.r).r,
	 texture(tex, texcoord + direction * distortion.g).g,
	 texture(tex, texcoord + direction * distortion.b).b
	);
}

void main()
{
	vec2 uv = -iTexcoord + vec2(1.0f);
	vec2 texelsize = 1.0f / vec2(textureSize(uColorTexture,0));

	// Vector to image center
	vec2 ghostVec = (vec2(0.5f) - uv) * uGhostDispers;
	// Vectors to sample the image (chromatic distortion)
	vec3 dDistortion = vec3(-texelsize.x * uDistortion, 0.0f, texelsize.x * uDistortion);
   	vec2 dDirection = normalize(ghostVec);

	// Sample ghosts
	vec3 res = vec3(0.0f);
	for(int i=0;i<uGhostSamples;i++)
	{
		vec2 off = fract(uv + ghostVec * float(i));
		// Allow only bright spots from the center of the image
		float weight = length(vec2(0.5f) - off) / length(vec2(0.5f));
      	weight = pow(1.0 - weight, 10.0);
		res += TextureDistorted(uColorTexture,off,dDirection,dDistortion) * weight;
	}
	// Radially sample color from 1d texture
	// res *= texture(uLensColor, length(vec2(0.5f) - uv) / length(vec2(0.5f)));

	// Halo
   	vec2 haloVec = normalize(ghostVec) * uHaloWidth;
   	float weight = length(vec2(0.5f) - fract(uv + haloVec)) / length(vec2(0.5f));
   	weight = pow(1.0 - weight, 5.0);
  	res += TextureDistorted(uColorTexture, uv + haloVec,dDirection,dDistortion) * weight;

	oColor = vec4(res,1.0f);
}