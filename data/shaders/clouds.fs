#version 430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform sampler2D uLutTexture;
uniform float uScaleFactor;
uniform vec3 uSundir;
uniform int uScatSamples;
uniform float uSampleDist;

in vec2 iTexcoord;
in vec3 iWPos;
in vec3 iClipPos;
in float iLogz;

out vec4 oColor;

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


/*
	Fractional Brownian motion [0,1] 8 octaves
*/
float Fbm8(vec3 pos) 
{

	vec3 q = pos;
	float f;
    f  = 0.50000*VNoise( q ); 	q = q*2.02;
    f += 0.25000*VNoise( q ); 	q = q*2.03;
    f += 0.12500*VNoise( q ); 	q = q*2.01;
    f += 0.06250*VNoise( q ); 	q = q*2.03;
    f += 0.03125*VNoise( q ); 	q = q*2.02;
    f += 0.015625*VNoise( q );	q = q*2.01;
    f += 0.0078125*VNoise( q );	q = q*2.03;
    f += 0.00390625*VNoise( q );
	return f;
}

/*
	Check the thikness of the clouds by performing 
	more samples "into the cloud".
*/
float Scattering()
{
	vec2 uWind = vec2(0.05f,0.05f) * uTime;

	float step = uSampleDist / float(uScatSamples);
	float n = 0.0f;
	vec3 pos = uScaleFactor*iWPos + vec3(uWind.x,0.0f,uWind.y);
	pos.x *= 1.25f;	// strech the cloud, realistic?
	pos.y = 0.0f;
	for(int i=0;i<uScatSamples;i++)
	{
		n += pow(Fbm2(pos),2.0f);
		pos.y += step;
	}
	n /= float(uScatSamples);
	return n;
}

float GetFade(float dist)
{
	float uFadeDist = 100200.0f;
	float d = clamp(dist,0.0f,uFadeDist);
	d = d / uFadeDist;
	d = clamp(d,0.0f,1.0f);
	return (d - 1.0f) * -1.0f;
}

void main()
{
	vec3 sunDir = vec3(0.4f,0.6f,0.0f);
	vec2 uWind = vec2(0.05f,0.05f) * uTime;
	float camDist = distance(uCampos,iWPos);
	float fade = GetFade(camDist);
	if(fade <= 0.0f)discard;
    float n = Scattering();

    // Clouds shape
    //vec3 csPos = (uScaleFactor*0.5f)*iWPos;
    //csPos.y = 0.0f;
    //float cs = Fbm2(csPos);

    // Cloud colors
	vec3 uCloudDark = vec3(0.57f,0.63f,0.7f);
    vec3 uCloudMorning = vec3(0.95f,0.95f,0.95f);
    vec3 uCloudSunset = vec3(0.91f,0.65f,0.505f);
	vec3 uCloudBright = mix(uCloudSunset,uCloudMorning,sqrt(uSundir.y));
	vec3 cloudColor = mix(uCloudBright,uCloudDark,n);
	oColor = vec4(cloudColor,n);
	oColor.a *= fade;

    // Logarithmic z-buffer
    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}



