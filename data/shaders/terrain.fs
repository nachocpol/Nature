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

uniform sampler2D uGrassTexture;
uniform sampler2D uCliffTexture;
uniform sampler2D uSplatTexture;
uniform sampler2D uHeightMap;
uniform sampler2D uLutTexture;
uniform sampler2D uSnowTexture;
uniform sampler2D uNormalTexture;
uniform vec3 uSunPosition;
uniform float uTiling1;
uniform float uTiling2;

in vec2 iTexcoord;
in vec3 iPosition;
in vec3 iColor;
in vec3 iSecondaryColor;
in vec3 iNormal;
in vec3 iClipPos;
in float iLogz;

out vec4 oColor;

vec4 Hash4( vec2 p ) 
{ 
	return fract(sin(vec4( 1.0+dot(p,vec2(37.0,17.0)), 
                                              2.0+dot(p,vec2(11.0,47.0)),
                                              3.0+dot(p,vec2(41.0,29.0)),
                                              4.0+dot(p,vec2(23.0,31.0))))*103.0); 
}

/*
	Value noise using lut texture [0,1]
*/
float VNoise( in vec3 x )
{
    vec3 p = floor(x.xzy);
    vec3 f = fract(x.xzy);
	vec3 f2 = f*f; 
	f = f*f2*(10.0-15.0*f+6.0*f2);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( uLutTexture, (uv+0.5)/256.0, -100.0 ).ba;
    float n = mix( rg.y, rg.x, f.z )-.5;
	return n + 1.0 * 0.5;
}

/*
	Fractional Brownian motion [0,1] 2 octaves
*/
float Fbm2(vec3 pos) 
{
	vec3 q = pos;
	float f;
    f  = 0.50000*VNoise( q ); q = q*2.02;
    f += 0.25000*VNoise( q );;
	return f;
}

/*
	Fractional Brownian motion [0,1] 5 octaves
*/
float Fbm5(vec3 pos) 
{

	vec3 q = pos;
	float f;
    f  = 0.50000*VNoise( q ); q = q*2.02;
    f += 0.25000*VNoise( q ); q = q*2.03;
    f += 0.12500*VNoise( q ); q = q*2.01;
    f += 0.06250*VNoise( q ); q = q*2.02;
    f += 0.03125*VNoise( q );
	return f;
}

float CloudsShadowing()
{
	vec2 w = vec2(0.05f,0.05f) * uTime;
	vec3 p = vec3(iPosition.x,250.0f,iPosition.z);
	p *= 0.0015f;
	p.xz += w;
	return (Fbm2(p)-1.0f)*-1.0f;
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

vec3 GetBaseColor()
{
	vec3 splat = texture(uSplatTexture,iTexcoord).xyz;

	vec3 grass = texture(uGrassTexture,iTexcoord * uTiling1).xyz;
	vec3 cliff = texture(uCliffTexture,iTexcoord * uTiling2).xyz;
    vec3 snow = texture(uSnowTexture,iTexcoord * uTiling1).xyz;
	
    vec3 finalColor = vec3(0.0f);
    finalColor = mix(finalColor,grass,splat.g);
    finalColor = mix(finalColor,cliff,splat.b);
    finalColor = mix(finalColor,snow,splat.r);  
    return finalColor;
}

vec3 GetAtmosphereColor(vec3 base)
{
    return iColor + base * iSecondaryColor;
}

vec3 ColorCorrect(vec3 color,float exposure)
{
    // Reinhard tone map + gamma correction
    const float gamma = 2.2f;
    vec3 mapped = vec3(1.0f) - exp(-color * exposure);
    return pow(mapped, vec3(1.0f / gamma));
}

void main()
{
    // Load normal and hack to work with world machine normals
    vec3 normal = texture(uNormalTexture,iTexcoord).xzy;
    normal.z = (normal.z - 1.0f) * -1.0f;

    vec3 base = GetBaseColor();
    vec3 baseAtm = GetAtmosphereColor(base);

    // Lambert
    float l = max(dot(normalize(normal),uSunPosition),0.1f);
    oColor = vec4(ColorCorrect(baseAtm,0.2f) * l,1.0f) * CloudsShadowing();

    // Logarithmic z-buffer
    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
    //const float C = 1.0;
    //const float offset = 1.0;
    //gl_FragDepth = (log(C * iCPos.z + offset) / log(C * uCamfar + offset));
}   


