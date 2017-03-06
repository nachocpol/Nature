#version 430 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexcoord;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
};

uniform mat4 uModel;
uniform vec2 uChunkPos;
uniform vec4 uClipPlane;
uniform sampler2D uHeightMap;

out vec2 iTexcoord;
out vec3 iPosition;

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

vec2 GetUv()
{
	float uHeightMapSize = 2048.0f;;
	vec2 p = (uModel * vec4(aPosition,1.0f)).xz;
	return p/uHeightMapSize;
}

void main()
{
	vec2 uv = GetUv();
	iTexcoord = uv;
	iPosition = (uModel * vec4(aPosition,1.0f)).xyz;
	iPosition.y = Fbm(vec3(uv.x,0.0f,uv.y)*100.0f,8,0.1f) * 50.0f;
	gl_ClipDistance[0] = dot(vec4(iPosition,1.0f),uClipPlane);
	gl_Position = uProjection * uView * vec4(iPosition,1.0f);
}