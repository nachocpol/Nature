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
in vec3[] gPosition;
uniform float mLodRange;
uniform float mNearLodRange;

layout (triangle_strip,max_vertices = 21) out;
out float iLogz;
flat out int iLod;

float GetWind(float s)
{
	float t = (uTime * 0.2f) + s;
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

mat4 YRotMatrix(float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	return mat4
	(
		c,		0.0f,	-s,		0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		s,		0.0f,	c,		0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	);
}

void GenBladeQuad(float Fcoef,vec3 b,vec3 t,int lod,mat4 model)
{
	// Frist quad 		
	vec3 bl = gPosition[0] + vec3(-b.x,b.y,b.z);
	gl_Position = uProjection * uView * model * vec4(bl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 br = gPosition[0] + vec3( b.x,b.y,b.z);
	gl_Position = uProjection * uView * model * vec4(br,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 tr = gPosition[0] + vec3( t.x,t.y,t.z);
	gl_Position = uProjection * uView * model * vec4(tr,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	// Second quad
	gl_Position = uProjection * uView * model * vec4(bl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	gl_Position = uProjection * uView * model * vec4(tr,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 tl = gPosition[0] + vec3(-t.x,t.y,t.z);
	gl_Position = uProjection * uView * model * vec4(tl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();
}

void GenBladeTop(float Fcoef,vec3 b,vec3 t,int lod,mat4 model)
{
	vec3 bl = gPosition[0] + vec3(-b.x,b.y,b.z);
	gl_Position = uProjection * uView * model * vec4(bl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 br = gPosition[0] + vec3( b.x,b.y,b.z);
	gl_Position = uProjection * uView * model * vec4(br,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 tr = gPosition[0] + vec3( t.x,t.y,t.z);
	gl_Position = uProjection * uView * model * vec4(tr,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();
}

void main()
{
	// Blade proportions
	const float bladeHalfW = 0.3f;
	const float bladeHeight = 2.5f;
	float Fcoef = 2.0 / log2(uCamfar + 1.0);

	// LOD min 1 max 4
	float camDist = distance(uCampos,gPosition[0]);
	float d = (camDist - mNearLodRange) / mLodRange;
	if(camDist > mLodRange + 300.0f)return;
	d = clamp(d,0.0f,1.0f);
	int maxQuads = int(mix(4,1,d));
	float curH = bladeHeight / maxQuads;

	// Mods for wind and width
	float bDelta = 1.0f / float(maxQuads);
	float widthAcum = 1.0f;
	float widthAcum2 = widthAcum - bDelta;
	float windAcum = 0.0f;
	float windAcum2 = windAcum + bDelta;

	// Rotation
	mat4 yRot = YRotMatrix(0.1f);

	// Blade body
	for(int i=0;i<maxQuads-1;i++)
	{
		float wind = GetWind(gPosition[0].x);
		GenBladeQuad
		(
			Fcoef,
			vec3(bladeHalfW * widthAcum ,i * curH		,wind * windAcum),
			vec3(bladeHalfW * widthAcum2,(i + 1) * curH	,wind * windAcum2),
			maxQuads,
			yRot
		);
		widthAcum -= bDelta;
		widthAcum2 -= bDelta;
		windAcum += bDelta;
		windAcum2 += bDelta;
	}

	// Blade top
	float wind = GetWind(gPosition[0].x);
	GenBladeTop
	(
		Fcoef,
		vec3(bladeHalfW * widthAcum,(maxQuads - 1) * curH	,wind * windAcum),
		vec3(0.0f					, maxQuads * curH	 	,wind * windAcum2),
		maxQuads,
		yRot
	);
	EndPrimitive();
}