#version  430 core

layout(std140)uniform uPass
{
	mat4 uView;
	mat4 uProjection;
	vec3 uCampos;
	float uTime;
	float uCamnear;
	float uCamfar;
};

uniform vec3 uSunPosition;

in vec3 iPosition;

out vec4 oColor;

/*
	Simple 2d pseudo random
*/
vec2 rand2(vec2 p)
{
    p = vec2(dot(p, vec2(12.9898,78.233)), dot(p, vec2(26.65125, 83.054543))); 
    return fract(sin(p) * 43758.5453);
}

/*
	1D pseudo random
*/
float rand(vec2 p)
{
    return fract(sin(dot(p.xy ,vec2(54.90898,18.233))) * 4337.5453);
}

float Stars(in vec2 x, float numCells, float size, float br)
{
    vec2 n = x * numCells;
    vec2 f = floor(n);

	float d = 1.0e10;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            vec2 g = f + vec2(float(i), float(j));
			g = n - g - rand2(mod(g, numCells)) + rand(g);
            // Control size
            g *= 1. / (numCells * size);
			d = min(d, dot(g, g));
        }
    }

    return br * (smoothstep(.95, 1., (1. - sqrt(d))));
}

void main()
{
	vec3 toAtmosphere = normalize(iPosition - vec3(0.0f));
	vec3 toSun = normalize(normalize(uSunPosition) - vec3(0.0f));

	// Sun
	float sunHeight = toSun.y;
	
	// Nighttime sky
	if(sunHeight <= 0.0f)
	{
		oColor.xyz = vec3(0.0,0.0,0.0);
		vec2 scaledToAtm = toAtmosphere.xz * 20.0;
		if(toAtmosphere.y > 0.1)
		{
			oColor.xyz += vec3(Stars(scaledToAtm,8.0,0.05,0.5));
		}
	}
	// Daytime sky
	else
	{
		float sunFactor = pow(max(dot(toAtmosphere,toSun),0.0f),3800.0f);
		float sunIntensity = mix(150.0f,300.0f,sunHeight);
		vec3 sunColor = vec3(0.95f,0.7f,0.1f) * sunFactor * sunIntensity;
		
		// Atmosphere
		// Basic mix of white at the horizon and bluish
		vec3 lowAtmCol = mix(vec3(0.95f,0.7f,0.1f),vec3(1.0f,1.0f,1.0f),sunHeight);
		vec3 atmosphereColor = mix(lowAtmCol,vec3(0.1f,0.4f,0.9f),pow(iPosition.y,0.35f));
		// Dark during the night
		atmosphereColor = mix(vec3(0.0f),atmosphereColor,pow(sunHeight,0.5f));

		oColor = vec4(/*sunColor +*/ atmosphereColor,1.0f);
	}

	if(iPosition.y <= 0.0f)oColor = vec4(0.2f);
}