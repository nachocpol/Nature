#version 430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
};
uniform sampler2D uLutTexture;
uniform float uScaleFactor;

in vec2 iTexcoord;
in vec3 iWPos;

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
float Scattering(vec3 ro,vec3 rd)
{
    const int samples = 8;
    float sampleDist = 0.5f;
    float acum = 0.0f;
    for(int i = 0; i < samples; i++)
    {
        float idx = float(i) / float(samples);
        acum += Fbm5(ro + (rd * (idx * sampleDist)));
    }
    return acum / float(samples);
}

float GetFade(float dist)
{
	float uFadeDist = 100200.0f;
	float d = clamp(dist,0.0f,uFadeDist);
	d = d / uFadeDist;
	d = clamp(d,0.0f,1.0f);
	//return 1.0f;
	return (d - 1.0f) * -1.0f;
}

void main()
{
	vec3 sunDir = vec3(0.4f,0.6f,0.0f);
	vec2 w = vec2(0.25f,0.25f) * uTime;

	float camDist = distance(uCampos,iWPos);
	//if(camDist > 4500.0f)discard;
	
	float n = 0.0f;
    vec3 q = uScaleFactor*iWPos + vec3(w.x,0.0f,w.y);
    q.y = 0.0f;
    n = Fbm5(q);

	vec3 uCloudBrig = vec3(0.95f,0.95f,0.95f);//morning
	uCloudBrig = vec3(0.91f,0.65f,0.505f);//sunset
	vec3 uCloudDark = vec3(0.57f,0.63f,0.7f);
	vec3 cloudColor = mix(uCloudBrig,uCloudDark,n);
	oColor = vec4(cloudColor,(n - 1.0f) * -1.0f);
	oColor.a *= GetFade(camDist);
	//oColor = vec4(n);
}



