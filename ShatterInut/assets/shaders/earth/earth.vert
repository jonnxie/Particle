#version 450

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;

layout(set = 2,binding = 0) uniform UniformEarthObject{
	double radius;
} earth;

layout (location = 0) in dvec2 inCoor;

layout (location = 0) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	double length = earth.radius * cos(radians(float(inCoor.y)));
	dvec3 position;
	position.x = length * cos(radians(float(inCoor.x)));
	position.y = length * sin(radians(float(inCoor.x)));
	position.z = earth.radius * sin(radians(float(inCoor.y)));

	outUV = vec2(inCoor) / vec2(180.0f, 90.0f);
	outUV += 1.0f;
	outUV /= 2.0f;

	gl_Position = vec4(c.proj * c.view * m.model * dvec4(position, 1.0f));
}
