#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	mat4 model = m.model;
	model[2][2] *= -1;
	outWorldPos = 	vec3(m.model * vec4(inPos, 1.0f));
	outNormal 	= 	mat3(m.model) * inNormal;
	gl_Position =  	c.proj * c.view * vec4(outWorldPos, 1.0f);
}
