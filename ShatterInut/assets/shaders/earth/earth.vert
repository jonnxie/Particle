#version 450

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;

layout(set = 2,binding = 0) uniform UniformEarthObject{
	vec3 center;
	float radius;
} earth;

layout (location = 0) in vec2 inCoor;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outWorldPos;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	float length = earth.radius * cos(inCoor.x);
	vec3 position = {length * cos(inCoor.x), length * sin(inCoor.x), earth.radius * sin(inCoor.y)};

	outNormal = normalize(position);
	outUV = inCoor / vec2(180.0f, 90.0f);
	outUV += 1.0f;
	outUV /= 2.0f;

	position += earth.center;
	outWorldPos = (m.model * vec4(position, 1.0f)).rgb ;
	gl_Position = c.proj * c.view * vec4(position, 1.0f);
}
