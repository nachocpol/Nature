#version 430 core
#define PI 3.141517

layout (points) in;
layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};
in vec2[] gPosition;
uniform float mLodRange;
uniform float mNearLodRange;
uniform sampler2D uHeightMap;

layout (triangle_strip,max_vertices = 3) out;
out float iLogz;
flat out int iLod;
out vec3 iPosition;
out vec3 iWPos;

float Hash(vec3 p)
{
    p  = fract( p*0.3183099+.1 );
	p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float GetWind(float s)
{
	float t = (uTime * 0.08f) + s;
	return 	(cos(t * PI) * cos(t * PI)) * 
			cos(t * 3.0f * PI) *
			cos(t * 5.0f * PI) *
			0.5f +
			sin(t * 25 * PI) * 
			0.05f;
}

mat4 RotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

mat4 TranslateMatrix(vec3 t)
{
	return mat4
	(
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		t.x,t.y,t.z,1.0f
	);
}

mat4 ScaleMatrix(vec3 s)
{
	return mat4
	(
		s.x,0.0f,0.0f,0.0f,
		0.0f,s.y,0.0f,0.0f,
		0.0f,0.0f,s.z,0.0f,
		0.0f,0.0f,0.0f,1.0f
	);
}

void GenVertex(float Fcoef,vec3 v,mat4 m,int lod)
{
	iPosition = v;
	iWPos = (m * vec4(v,1.0f)).xyz;
	gl_Position = uProjection * uView * vec4(iWPos,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();
}

void GenBladeQuad(float Fcoef,vec3 b,vec3 t,int lod,mat4 model)
{
	// Frist quad 	
	vec3 bl = vec3(-b.x,b.y,b.z);
	GenVertex(Fcoef,bl,model,lod);
	vec3 br = vec3( b.x,b.y,b.z);
	GenVertex(Fcoef,br,model,lod);
	vec3 tr = vec3( t.x,t.y,t.z);
	GenVertex(Fcoef,tr,model,lod);

	// Second quad
	GenVertex(Fcoef,bl,model,lod);
	GenVertex(Fcoef,tr,model,lod);
	vec3 tl = vec3(-t.x,t.y,t.z);
	GenVertex(Fcoef,tl,model,lod);
}

void GenBladeTop(float Fcoef,vec3 b,vec3 t,int lod,mat4 model)
{
	vec3 bl = vec3(-b.x,b.y,b.z);
	GenVertex(Fcoef,bl,model,lod);

	vec3 br = vec3( b.x,b.y,b.z);
	GenVertex(Fcoef,br,model,lod);

	vec3 tr = vec3( t.x,t.y,t.z);
	GenVertex(Fcoef,tr,model,lod);
}

float GetHeight()
{
	// 9.0f = Map scale (so we get the hmap values)
	vec2 pos = gPosition[0].xy / 9.0f;
	pos /= 2048.0f;
	return texture(uHeightMap,pos).x * 200.0f * 9.0f;
}

void main()
{
	// Blade proportions
	const float bladeHalfW = 0.04f;
	const float bladeHeight = 1.0f;
	float Fcoef = 2.0 / log2(uCamfar + 1.0);
	float bladeY = GetHeight();
	//bladeY = 100.0;

	// LOD min 1 max 4
	float camDist = distance(uCampos,vec3(gPosition[0].x,bladeY,gPosition[0].y));
	float d = (camDist - mNearLodRange) / mLodRange;
	if(camDist > mLodRange + 300.0f)
	{
		iLod = 0;
		EndPrimitive();
		return;
	}
	d = clamp(d,0.0f,1.0f);
	int maxQuads = /*int(mix(1,1,d))*/ 1;
	float curH = bladeHeight / maxQuads;

	// Mods for wind and width
	float bDelta = 1.0f / float(maxQuads);
	float widthAcum = 1.0f;
	float widthAcum2 = widthAcum - bDelta;
	float windAcum = 0.0f;
	float windAcum2 = windAcum + bDelta;

	// Transform
	mat4 rot = RotationMatrix(vec3(0.0f,1.0f,0.0f),gPosition[0].x * gPosition[0].y);
	mat4 trans = TranslateMatrix(vec3(gPosition[0].x,bladeY,gPosition[0].y));
	mat4 scale = ScaleMatrix(vec3(Hash(vec3(gPosition[0].x,bladeY,gPosition[0].y)) + 1.0f));
	mat4 m = trans * rot * scale;

	// Blade body
	/*
	for(int i=0;i<maxQuads-1;i++)
	{
		float wind = GetWind(gPosition[0].x);
		GenBladeQuad
		(
			Fcoef,
			vec3(bladeHalfW * sqrt(widthAcum),i * curH		,wind * windAcum),
			vec3(bladeHalfW * sqrt(widthAcum2),(i + 1) * curH	,wind * windAcum2),
			maxQuads,
			m
		);
		widthAcum -= bDelta;
		widthAcum2 -= bDelta;
		windAcum += bDelta;
		windAcum2 += bDelta;
	}
	*/

	// Blade top
	float wind = GetWind(gPosition[0].x);
	GenBladeTop
	(
		Fcoef,
		vec3(bladeHalfW * sqrt(widthAcum),(maxQuads - 1) * curH	,wind * windAcum),
		vec3(0.0f					, maxQuads * curH	 	,wind * windAcum2),
		maxQuads,
		m
	);
	EndPrimitive();
}