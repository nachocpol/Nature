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
uniform sampler2D uGrassBlades;
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

mat3 GetTBNMatrix(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
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

#define MAX_RAYDEPTH 5                                     //Number of iterations.
#define PLANE_NUM 32.0                                      //Number of grass slice grid planes per unit in tangent space.
#define PLANE_NUM_INV (1.0/PLANE_NUM)
#define PLANE_NUM_INV_DIV2 (PLANE_NUM_INV/2)
#define GRASS_SLICE_NUM 8                                   // Number of grass slices in texture grassblades.
#define GRASS_SLICE_NUM_INV (1.0/GRASS_SLICE_NUM)
#define GRASS_SLICE_NUM_INV_DIV2 (GRASS_SLICE_NUM_INV/2)
#define GRASSDEPTH GRASS_SLICE_NUM_INV                      //Depth set to inverse of number of grass slices so no stretching occurs.
#define TC1_TO_TC2_RATIO 8                                  //Ratio of texture coordinate set 1 to texture coordinate set 2, used for the animation lookup.
#define PREMULT (GRASS_SLICE_NUM_INV*PLANE_NUM)             //Saves a multiply in the shader.
#define AVERAGE_COLOR vec4(0.32156,0.513725,0.0941176,1.0)  //Used to fill remaining opacity, can be replaced by a texture lookup.

vec4 GetGrass(vec3 n)
{
    // Initialize increments/decrements and per fragment constants
    vec4 color = vec4(0.0,0.0,0.0,0.0);

    // Atributes into
    vec3 eyeDir = -normalize(uCampos - iPosition);
    mat3 tbn = GetTBNMatrix(n,eyeDir,iTexcoord * uTiling1);
    vec3 eyeDirTan = normalize(tbn * eyeDir);
    // oEyeDirTan = normalize(mul(TBNMatrix,eyeDirO)); // eye vector in tangent space

    vec2 plane_offset = vec2(0.0,0.0);                  
    vec3 rayEntry = vec3(iTexcoord.xy * uTiling1,0.0);
    float zOffset = 0.0;
    int zFlag = 1;

    // The signs of eyeDirTan determines if we increment or decrement along the tangent space axis
    // plane_correct, planemod and pre_dir_correct are used to avoid unneccessary if-conditions. 
    vec2 sign = vec2(sign(eyeDirTan.x),sign(eyeDirTan.y));  
    vec2 plane_correct = vec2((sign.x+1)*GRASS_SLICE_NUM_INV_DIV2,
                              (sign.y+1)*GRASS_SLICE_NUM_INV_DIV2);
    vec2 planemod = vec2(floor(rayEntry.x*PLANE_NUM)/PLANE_NUM,
                         floor(rayEntry.y*PLANE_NUM)/PLANE_NUM);
    vec2 pre_dir_correct = vec2((sign.x+1)*PLANE_NUM_INV_DIV2,
                                (sign.y+1)*PLANE_NUM_INV_DIV2);

    int hitcount;
    for(hitcount =0; hitcount < MAX_RAYDEPTH % (MAX_RAYDEPTH+1); hitcount++)    // %([MAX_RAYDEPTH]+1) speeds up compilation.
                                                                                // It may proof to be faster to early exit this loop
                                                                                // depending on the hardware used.
    {
        // Calculate positions of the intersections with the next grid 
        // planes on the u,v tangent space axis independently.
        vec2 dir_correct = vec2(sign.x*plane_offset.x+pre_dir_correct.x,
                                sign.y*plane_offset.y+pre_dir_correct.y);           
        vec2 distance = vec2((planemod.x + dir_correct.x - rayEntry.x)/(eyeDirTan.x),
                             (planemod.y + dir_correct.y - rayEntry.y)/(eyeDirTan.y));
                    
        vec3 rayHitpointX = rayEntry + eyeDirTan *distance.x;   
        vec3 rayHitpointY = rayEntry + eyeDirTan *distance.y;
        
        // Check if we hit the ground. If so, calculate the intersection and 
        // look up the ground texture and blend colors.
        if ((rayHitpointX.z <= -GRASSDEPTH)&& (rayHitpointY.z <= -GRASSDEPTH))  
        {
            float distanceZ = (-GRASSDEPTH)/eyeDirTan.z; // rayEntry.z is 0.0 anyway 

            vec3 rayHitpointZ = rayEntry + eyeDirTan * distanceZ;
            vec2 orthoLookupZ = vec2(rayHitpointZ.x,rayHitpointZ.y);
                        
            color = (color)+((1.0-color.w)); /* * tex2D(ground,orthoLookupZ));*/
            if(zFlag == 1) zOffset = distanceZ;  // write the distance from rayEntry to intersection
            zFlag = 0;                          // Early exit here if faster.     
        }  
        else
        {
            vec2 orthoLookup; //Will contain texture lookup coordinates for grassblades texture.

            //check if we hit a u or v plane, calculate lookup accordingly with wind shear displacement.
            if(distance.x <= distance.y)
            {
                //vec4 windX = (tex2D(windnoise,texCoord2+rayHitpointX.xy/TC1_TO_TC2_RATIO)-0.5)/2;
                vec4 windX = vec4(1.0f);
                float lookupX = -(rayHitpointX.z+(planemod.x+sign.x*plane_offset.x)*PREMULT)-plane_correct.x;
                orthoLookup=vec2(rayHitpointX.y+windX.x*(GRASSDEPTH+rayHitpointX.z),lookupX); 
                
                plane_offset.x += PLANE_NUM_INV; // increment/decrement to next grid plane on u axis
                if(zFlag == 1) zOffset = distance.x;
            }
            else 
            {
                //vec4 windY = (tex2D(windnoise,texCoord2+rayHitpointY.xy/TC1_TO_TC2_RATIO)-0.5)/2;
                vec4 windY = vec4(1.0f);

                float lookupY = -(rayHitpointY.z+(planemod.y+sign.y*plane_offset.y)*PREMULT)-plane_correct.y;
                orthoLookup = vec2(rayHitpointY.x+windY.y*(GRASSDEPTH+rayHitpointY.z) ,lookupY);
            
                plane_offset.y += PLANE_NUM_INV;  // increment/decrement to next grid plane on v axis
                if(zFlag == 1) zOffset = distance.y;
            }
            color += (1.0-color.w)*texture(uGrassBlades,orthoLookup);
    
            if(color.w >= 0.49){zFlag = 0;} //Early exit here if faster.
        }
    }   
    color += (1.0-color.w)*AVERAGE_COLOR;   //  Fill remaining transparency in case there is some left. Can be replaced by a texture lookup
                                            //  into a fully opaque grass slice using orthoLookup.
    return color;
}

void main()
{
    // Load normal and hack to work with world machine normals
    vec3 normal = texture(uNormalTexture,iTexcoord).xzy;
    normal.z = (normal.z - 1.0f) * -1.0f;

    vec3 base = GetBaseColor();
    vec3 baseAtm = GetAtmosphereColor(base);

    // Lambert
    float l = max(dot(normalize(normal),uSunPosition),0.0f);
    oColor = vec4(baseAtm * l,1.0f) * CloudsShadowing();

    oColor = GetGrass(normalize(normal));

    // Logarithmic z-buffer
    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}   


