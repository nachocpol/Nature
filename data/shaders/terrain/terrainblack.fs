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

in vec2 iTexcoord;
in vec3 iPosition;
in float iLogz;

out vec4 oColor;

void main()
{
    oColor = vec4(0.0,0.0,0.0,1.0);

    float Fcoef_half = 0.5f * (2.0 / log2(uCamfar + 1.0));
    gl_FragDepth = log2(iLogz) * Fcoef_half;
}   


