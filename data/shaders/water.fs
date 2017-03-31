#version  430 core
#extension GL_EXT_gpu_shader4 : enable

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform sampler2D uReflectionTexture;
uniform sampler2D uRefractionTexture;
uniform sampler2D uDudvTexture;
uniform sampler2D uNormTexture;
uniform sampler2D uTerrainTexture;
uniform	float uWaveSize;
uniform	float uWaveStrength;
uniform	float uNormStrenght;//bigger smother
uniform	vec2 uWaveSpeed;
uniform	vec2 uWaveSpeed2;
uniform vec3 uSundir;
uniform float uWaterShinyFactor;
uniform vec3 uSpecColor;
uniform vec3 uWaterTint;
uniform float uWaterTintFactor;
uniform float uTerrainHeightScale;
uniform float uTerrainScale;

in vec2 iTexcoord;
in vec4 iCPos;
in vec3 iWPos;
in float iLogz;

out vec4 oColor;

float CalcFresnel(vec3 normal)
{
	vec3 toView = normalize(uCampos - iWPos);
	float f = max(dot(toView,normal),0.0);
	return f;
}

vec2 GetUv(float shore)
{
	vec2 samp = vec2(iTexcoord.x,iTexcoord.y);
	vec2 disp = texture(uDudvTexture,(samp + (uWaveSpeed * uTime)) * uWaveSize).xy;
	disp = disp * 2.0f - 1.0f;
	disp *= uWaveStrength * shore;
	return disp;
}

float GetDistanceToFloor()
{
	// Sample the terrain 4 times to overcome 
	// low res issue
	float tTexelSize = 1.0f / 2048.0f;
	vec4 samples;
	samples[0] = texture(uTerrainTexture,iTexcoord + vec2(-tTexelSize,-tTexelSize)).r;
	samples[1] = texture(uTerrainTexture,iTexcoord + vec2(tTexelSize,-tTexelSize)).r;
	samples[2] = texture(uTerrainTexture,iTexcoord + vec2(tTexelSize,tTexelSize)).r;
	samples[3] = texture(uTerrainTexture,iTexcoord + vec2(-tTexelSize,tTexelSize)).r;
	float sampleCur = texture(uTerrainTexture,iTexcoord).r;
	float tHeight = (samples[0] + samples[1] + samples[2] + samples[3] + sampleCur) * 0.2f;
	tHeight  = tHeight * uTerrainScale * uTerrainHeightScale;
	float curHeight = iWPos.y;

	return curHeight - tHeight;
}

float GetWaterFade(float dist)
{
	float uWaterShoreFade = 0.5;
	float fadeFactor = dist / uWaterShoreFade;
	fadeFactor = clamp(fadeFactor,0.0,1.0);
	return mix(0.0,1.0,fadeFactor);
	//return 1.0;
	//return clamp(pow(dist/uWaterShoreFade,1.0f),0.0f,1.0f);
}

vec3 GetNormal()
{
	vec2 uv = (iTexcoord + (uWaveSpeed * uTime)) * uWaveSize;
	vec2 uv2 = (iTexcoord + (uWaveSpeed2 * uTime)) * uWaveSize;

	vec3 fNorm;
	vec4 nMap = texture(uNormTexture,uv);
	vec4 nMap2= texture(uNormTexture,uv2);
	nMap += nMap2;
	fNorm.x = nMap.r * 2.0f - 1.0f;
	fNorm.y = nMap.b * uNormStrenght;
	fNorm.z = nMap.g * 2.0f - 1.0f;
	return normalize(fNorm);
}

vec4 GetWaterColor()
{
	// Uv screen space
	vec2 uv = iCPos.xy / iCPos.w;
	uv = uv * 0.5f + 0.5f;

	// Reduce displacement at the shore
	float distToFloor = GetDistanceToFloor();
	float shoreFact = clamp(distToFloor/50.0f,0.0f,1.0f);
	uv += GetUv(shoreFact);

	// Clamp to remove wavy edges on the screen
	uv.x = clamp(uv.x,0.001f,0.999f);
	uv.y = clamp(uv.y,0.001f,0.999f);
	
	vec3 reflectCol = texture(uReflectionTexture,vec2(uv.x,-uv.y)).xyz;
	vec3 refractCol = texture(uRefractionTexture,uv).xyz;

	// Fresnel
	vec3 wNorm = GetNormal();
	vec3 c =  mix(reflectCol,refractCol,CalcFresnel(vec3(0.0f,1.0f,0.0f)));

	// Water tint
	c = mix(c,uWaterTint,uWaterTintFactor);

	// Specular
	vec3 viewDir = normalize(uCampos - iWPos);
	vec3 halfVec = normalize(normalize(uSundir) + viewDir);
	float spec = pow(max(dot(wNorm,halfVec),0.0f),uWaterShinyFactor);
	vec3 specColor = uSpecColor * spec;

	// Fade the water at the shore
	return vec4(c + specColor,GetWaterFade(distToFloor));
}

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
	oColor = GetWaterColor();
    //oColor.xyz = GetFog(oColor.xyz,distance(uCampos,iWPos),uCampos,normalize(iWPos - uCampos));

    // Logarithmic z-buffer
    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}