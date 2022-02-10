#version 450

layout (location = 0) in vec3 inPos;

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;

layout(set = 2,binding = 0) uniform UniformCameraPObject{
	vec3 camPos;
} camera;

layout (location = 0) out vec3 outUVW;//OneDimensionTex

out gl_PerVertex
{
	vec4 gl_Position;
};

void main ()
{
	outUVW = inPos;
	outUVW.xy *= -1.0f;
	gl_Position = c.proj * c.view /*m.model */ * vec4(camera.camPos + inPos.xyz,1.0f);
	gl_Position.z = gl_Position.w;
}