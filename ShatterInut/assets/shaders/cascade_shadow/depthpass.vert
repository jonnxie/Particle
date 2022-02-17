#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

// todo: pass via specialization constant
#define SHADOW_MAP_CASCADE_COUNT 4

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
} m;

layout(set = 1, binding = 0) uniform CASCADE{
	uint cascadeIndex;
} c;

layout (set = 1, binding = 1) uniform UBO {
	mat4[SHADOW_MAP_CASCADE_COUNT] cascadeViewProjMat;
} ubo;

//layout (location = 0) out vec2 outUV;/

out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
//	outUV = inUV;/
	gl_Position =  ubo.cascadeViewProjMat[c.cascadeIndex] * m.model * vec4(inPos, 1.0);
}