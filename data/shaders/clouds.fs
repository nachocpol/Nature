#version 430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
};
uniform sampler2D uBaseTexture;
uniform sampler2D uNoiseTexture;
uniform sampler2D uDepthTexture;

in vec2 iTexcoord;
in vec3 iWPos;

out vec4 oColor;

/*
	Has function.
*/
float Hash(vec3 p) 
{
	p  = fract( p*0.3183099+.1 );
  	p *= 17.0;
  	return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
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

vec3 GetCamera(vec2 uv)
{
	vec3 rd;
	mat4 v = uView;
	v[3] = vec4(0.0f,0.0f,0.0f,1.0f);
	mat4 iView = inverse(v);
	mat4 iProj = inverse(uProjection);
	mat4 iVp = iView * iProj;
	rd = (iVp * vec4(uv.x,uv.y,0.0f,1.0f)).xyz;
	return normalize(rd);
}

/*
	Check the thikness of the clouds by performing 
	more samples "into the cloud".
*/
float Scattering(vec3 ro,vec3 rd)
{
    const int samples = 8;
    float sampleDist = 0.5f;
    float acum = 0.0f;
    for(int i = 0; i < samples; i++)
    {
        float idx = float(i) / float(samples);
        acum += Fbm(ro + (rd * (idx * sampleDist)),2,0.4f);
    }
    return acum / float(samples);
}

float GetFade(float dist)
{
	float uFadeDist = 1200.0f;
	float d = clamp(dist,0.0f,uFadeDist);
	d = d / uFadeDist;
	d = clamp(d,0.0f,1.0f);
	return (d - 1.0f) * -1.0f;
}


void main()
{
	vec3 sunDir = vec3(0.4f,0.6f,0.0f);
	vec2 w = vec2(0.25f,0.25f) * uTime;
	vec3 p = iWPos * 0.05f;

	float camDist = distance(uCampos,iWPos);
	if(camDist > 1500.0f)discard;
	
	// First layer
	p.x += w.x + 1000.0f; 
	p.y += w.y + 1000.0f;
	float n1 = Fbm(p,4,0.4f);

	// Second layer
	p.x += w.x + 1000.0f; 
	p.y -= w.y + 1000.0f;
	float n2 = Fbm(p,4,0.4f);

	float n = mix(n1,n2,0.5f);

	vec3 uCloudBrig = vec3(0.95f,0.95f,0.95f);//morning
	//uCloudBrig = vec3(0.91f,0.65f,0.505f);//sunset
	vec3 uCloudDark = vec3(0.57f,0.63f,0.7f);
	vec3 cloudColor = mix(uCloudBrig,uCloudDark,sqrt(n));

	oColor = vec4(cloudColor,(n - 1.0f) * -1.0f);
	/*
	vec2 uv = iTexcoord * 2.0f - 1.0f;
	vec3 rd = GetCamera(uv);
	vec3 ro = vec3(uCampos.x,uCampos.y,uCampos.z);

	// Make it uniforms
	vec3 uCloudBrig = vec3(0.95f,0.95f,0.95f);//morning
	uCloudBrig = vec3(0.91f,0.65f,0.505f);//sunset
	vec3 uCloudDark = vec3(0.57f,0.63f,0.7f);
	vec2 uWind = vec2(0.15f);

	// Set initial values of color
	vec3 cFinal = vec3(0.0f);
	vec3 baseColor = texture(uBaseTexture,iTexcoord).xyz;
	baseColor = vec3(0.0f,0.0f,0.0f);
	oColor = vec4(baseColor,0.0f);

	// Hack, if we have something in the current texel,dont draw 
	// anything. Do this with stencil?
	float curDepth = texture(uDepthTexture,iTexcoord).x;

	// Check cloud intersection
	Plane clouds;
	clouds.Point = vec3(0.0f,250.0f,0.0f);
	clouds.Normal = vec3(0.0f,1.0f,0.0f);
	float cDist = IPlane(ro,rd,clouds);
	if(cDist != 0.0f && curDepth >= 1.0f)
	{
		// Cloud position and wind
		vec3 cPos = ro + (rd * cDist);
		cPos /= 50.0f;
		cPos.xy += uTime * uWind;
		
		// Check cloud density
		float s = Scattering(cPos,rd);
		s = mix(0.0f,s,GetFade(cDist));
		cFinal = mix(baseColor,mix(uCloudBrig,uCloudDark,sqrt(s)) * 2.0f,s);
		oColor = vec4(cFinal,(s - 1.0f) * -1.0f);
	}
	// If we dont hit the cloud just output with alpha 0%
	*/
}


