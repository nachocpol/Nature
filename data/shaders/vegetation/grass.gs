#version 430 core

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

layout (triangle_strip,max_vertices = 21) out;
out float iLogz;
flat out int iLod;

void GenBladeQuad(float Fcoef,vec3 b,vec3 t,int lod)
{
	// Frist quad 		
	vec3 bl = gPosition[0] + vec3(-b.x,b.y,b.z);
	gl_Position = uProjection * uView * vec4(bl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 br = gPosition[0] + vec3( b.x,b.y,b.z);
	gl_Position = uProjection * uView * vec4(br,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 tr = gPosition[0] + vec3( t.x,t.y,t.z);
	gl_Position = uProjection * uView * vec4(tr,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	// Second quad
	gl_Position = uProjection * uView * vec4(bl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	gl_Position = uProjection * uView * vec4(tr,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 tl = gPosition[0] + vec3(-t.x,t.y,t.z);
	gl_Position = uProjection * uView * vec4(tl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();
}

void GenBladeTop(float Fcoef,vec3 b,vec3 t,int lod)
{
	vec3 bl = gPosition[0] + vec3(-b.x,b.y,b.z);
	gl_Position = uProjection * uView * vec4(bl,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 br = gPosition[0] + vec3( b.x,b.y,b.z);
	gl_Position = uProjection * uView * vec4(br,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();

	vec3 tr = gPosition[0] + vec3( t.x,t.y,t.z);
	gl_Position = uProjection * uView * vec4(tr,1.0f);
	iLogz = 1.0f + gl_Position.w;
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
	iLod = lod;
	EmitVertex();
}

void main()
{
	// Blade proportions
	const float bladeHalfW = 0.3f;
	const float bladeHeight = 2.0f;
	float Fcoef = 2.0 / log2(uCamfar + 1.0);

	// LOD min 1 max 4
	float d = (distance(uCampos,gPosition[0]) - 250.0f) / 500.0f;
	d = clamp(d,0.0f,1.0f);
	int maxQuads = int(mix(4,1,d));
	float curH = bladeHeight / maxQuads;

	for(int i=0;i<maxQuads-1;i++)
	{
		GenBladeQuad
		(
			Fcoef,
			vec3(bladeHalfW,i * curH,0.0f),
			vec3(bladeHalfW,(i + 1) * curH,0.0f),
			maxQuads
		);
	}
	GenBladeTop
	(
		Fcoef,
		vec3(bladeHalfW,(maxQuads - 1) * curH,0.0f),
		vec3(0.0f,		 maxQuads * curH	 ,0.0f),
		maxQuads
	);
	EndPrimitive();
}