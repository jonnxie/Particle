#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;

layout (location = 0) out vec2 outUV;

void main () 
{
	outUV = inUV;
	
	gl_Position = c.proj * c.view * m.model * vec4(inPos.xyz, 1.0);
}