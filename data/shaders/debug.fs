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
in vec3 iCPos;

out vec4 oColor;

void main()
{
	oColor = vec4(0.0f,0.0f,0.0f,1.0f);

	// Logarithmic z-buffer
    const float C = 1.0;
    const float offset = 1.0;
    gl_FragDepth = (log(C * iCPos.z + offset) / log(C * uCamfar + offset));
}