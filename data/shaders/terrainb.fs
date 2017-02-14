#version  430 core
#extension GL_EXT_gpu_shader4 : enable

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
};

uniform sampler2D uGrassTexture;
uniform sampler2D uCliffTexture;
uniform sampler2D uSplatTexture;
uniform sampler2D uHeightMap;

in vec2 iTexcoord;
in vec3 iPosition;

out vec4 oColor;

/*
	Hash function.
*/
float Hash(vec3 p) 
{
	p  = fract( p*0.3183099+.1 );
  	p *= 17.0;
  	return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

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


/*
	3D value noise.
*/
float Noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
  
    return mix(mix(mix( Hash(p+vec3(0,0,0)), 
                        Hash(p+vec3(1,0,0)),f.x),
                   mix( Hash(p+vec3(0,1,0)), 
                        Hash(p+vec3(1,1,0)),f.x),f.y),
               mix(mix( Hash(p+vec3(0,0,1)), 
                        Hash(p+vec3(1,0,1)),f.x),
                   mix( Hash(p+vec3(0,1,1)), 
                        Hash(p+vec3(1,1,1)),f.x),f.y),f.z);
}

/*
    Fractional Brownian motion [0,1]
*/
float Fbm(vec3 pos, int octaves, float persistence) 
{

    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;  
    for(int i=0;i<octaves;i++) 
    {
        total += Noise(pos * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    return total/maxValue;
}
/*
	Plane definition
*/
struct Plane
{
    vec3 Point;
    vec3 Normal;
};

/*
	If a ray intersects the provided plane it will return
	the distance to the plane (if not just 0).
*/
float IPlane( vec3 ro, vec3 rd, Plane plane)
{
    float hit = 0.0;
    float dotP = dot(rd,plane.Normal);
    if(dotP == 0.0)
    {
        return hit;
    }
    
    float distToHit = dot(plane.Point - ro, plane.Normal) / dotP;
    if(distToHit < 0.0)
    {
        return hit;
    }
    
    hit = distToHit;
    return hit;
}

float GetFade(float dist)
{
	float uFadeDist = 1200.0f;
	float d = clamp(dist,0.0f,uFadeDist);
	d = d / uFadeDist;
	d = clamp(d,0.0f,1.0f);
	return (d - 1.0f) * -1.0f;
}

float CloudsShadowing()
{
	float cFactor = 1.0f;

	// TO-DO:Aspect ratio
	vec3 rd = -(vec3(0.5f,-1.0f,0.0f));
	vec3 ro = iPosition;

	// Make it uniforms
	vec2 uWind = vec2(0.15f);

	// Check cloud intersection
	Plane clouds;
	clouds.Point = vec3(0.0f,250.0f,0.0f);
	clouds.Normal = vec3(0.0f,1.0f,0.0f);
	float cDist = IPlane(ro,rd,clouds);
	if(cDist != 0.0f)
	{
		// Cloud position and wind
		vec3 cPos = ro + (rd * cDist);
		cPos /= 50.0f;
		cPos.xy += uTime * uWind;

		cFactor -= Fbm(cPos,2,0.65f);
	}
	cFactor = max(cFactor,0.2f);
	return cFactor;
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
	float tiling1 = 200.0f;
	float tiling2 = 100.0f;
	vec3 splat = texture(uSplatTexture,iTexcoord).xyz;
	vec3 grass = texture2DNoTile(uGrassTexture,iTexcoord * tiling1).xyz;
	vec3 cliff = texture2DNoTile(uCliffTexture,iTexcoord * tiling2).xyz;
	return mix(grass,cliff,sqrt(splat.z));
}

void main()
{
	//oColor = texture(uHeightMap,iTexcoord);
	oColor = vec4(iTexcoord.xy,0.0f,1.0f);
	//oColor = vec4(GetBaseColor(),1.0f) * CloudsShadowing();
}

