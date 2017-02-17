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
uniform sampler2D uRefractDepth;
uniform sampler2D uNormTexture;

in vec2 iTexcoord;
in vec4 iCPos;
in vec3 iWPos;

out vec4 oColor;

/*
	4 component hash
*/
vec4 Hash4( vec2 p ) 
{ 
	return fract(sin(vec4( 1.0+dot(p,vec2(37.0,17.0)), 
                                              2.0+dot(p,vec2(11.0,47.0)),
                                              3.0+dot(p,vec2(41.0,29.0)),
                                              4.0+dot(p,vec2(23.0,31.0))))*103.0); 
}

vec4 texture2DNoTile( sampler2D samp, in vec2 uv )
{
	vec2 iuv = floor( uv );
    vec2 fuv = fract( uv );

    // generate per-tile transform
    vec4 ofa = Hash4( iuv + vec2(0.0,0.0) );
    vec4 ofb = Hash4( iuv + vec2(1.0,0.0) );
    vec4 ofc = Hash4( iuv + vec2(0.0,1.0) );
    vec4 ofd = Hash4( iuv + vec2(1.0,1.0) );
    
    vec2 ddx = dFdx( uv );
    vec2 ddy = dFdy( uv );

    // transform per-tile uvs
    ofa.zw = sign(ofa.zw-0.5);
    ofb.zw = sign(ofb.zw-0.5);
    ofc.zw = sign(ofc.zw-0.5);
    ofd.zw = sign(ofd.zw-0.5);
    
    // uv's, and derivarives (for correct mipmapping)
    vec2 uva = uv*ofa.zw + ofa.xy; vec2 ddxa = ddx*ofa.zw; vec2 ddya = ddy*ofa.zw;
    vec2 uvb = uv*ofb.zw + ofb.xy; vec2 ddxb = ddx*ofb.zw; vec2 ddyb = ddy*ofb.zw;
    vec2 uvc = uv*ofc.zw + ofc.xy; vec2 ddxc = ddx*ofc.zw; vec2 ddyc = ddy*ofc.zw;
    vec2 uvd = uv*ofd.zw + ofd.xy; vec2 ddxd = ddx*ofd.zw; vec2 ddyd = ddy*ofd.zw;
        
    // fetch and blend
    vec2 b = smoothstep(0.25,0.75,fuv);
    
    return mix( mix( texture2DGrad( samp, uva, ddxa, ddya ), 
                     texture2DGrad( samp, uvb, ddxb, ddyb ), b.x ), 
                mix( texture2DGrad( samp, uvc, ddxc, ddyc ),
                     texture2DGrad( samp, uvd, ddxd, ddyd ), b.x), b.y );
}

float CalcFresnel(vec3 normal)
{
	vec3 toView = normalize(uCampos - iWPos);
	float f = max(dot(toView,normal),0.0);
	return f;
}

vec2 GetUv(float shore)
{
	float uWaveSize = 60.0f;
	float uWaveStrength = 0.02f;
	vec2 uWaveSpeed = vec2(0.001f,0.0005f) * uTime;

	vec2 samp = vec2(iTexcoord.x,iTexcoord.y);
	vec2 disp = texture(uDudvTexture,(samp + uWaveSpeed) * uWaveSize).xy;
	disp = disp * 2.0f - 1.0f;
	disp *= uWaveStrength * shore;
	return disp;
}

float GetDistanceToFloor(vec2 uv)
{
	float sceneDepth = texture(uRefractDepth,uv).x;
	float curDepth = gl_FragCoord.z;

	//Calculate distances (water-floor-camera)
	float dCamFloor = 2.0 * uCamnear * uCamfar /  ( uCamfar + uCamnear - (2.0 * sceneDepth - 1.0)*(uCamfar - uCamnear));
	float dCamWat = 2.0 * uCamnear * uCamfar /  ( uCamfar + uCamnear - (2.0 * curDepth - 1.0)*(uCamfar - uCamnear));
	return dCamFloor - dCamWat;
}

float GetWaterFade(float dist)
{
	float uWaterShoreFade = 2.0f;
	return clamp(dist/uWaterShoreFade,0.0f,1.0f);
}

vec3 GetNormal()
{
	float uWaveSize = 60.0f;
	float uNormStrenght = 1.0f;//bigger smother
	vec2 uWaveSpeed = vec2(0.001f,0.0005f) * uTime;
	vec2 uWaveSpeed2 = vec2(-0.001f,0.0005f) * uTime;

	vec2 uv = (iTexcoord + uWaveSpeed) * uWaveSize;
	vec2 uv2 = (iTexcoord + uWaveSpeed2) * uWaveSize;

	vec3 fNorm;
	vec4 nMap = texture2DNoTile(uNormTexture,uv);
	vec4 nMap2= texture2DNoTile(uNormTexture,uv2);
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
	float distToFloor = GetDistanceToFloor(uv);
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
	vec3 uWaterTint = vec3(0.23f,0.35f,0.57f);
	float uWaterTintFactor = 0.0f;
	c = mix(c,uWaterTint,uWaterTintFactor);

	// Specular
	vec3 uSunDir = -vec3(0.5f,-0.5f,0.0f);
	float uWaterShinyFactor = 30.0f;
	vec3 viewDir = normalize(uCampos - iWPos);
	vec3 halfVec = normalize(uSunDir + viewDir);
	float spec = pow(max(dot(wNorm,halfVec),0.0f),uWaterShinyFactor);
	vec3 specColor = vec3(0.1f,0.1f,0.1f) * spec;

	// Fade the water at the shore
	return vec4(c + spec,GetWaterFade(distToFloor));
}

void main()
{
	oColor = GetWaterColor();
}