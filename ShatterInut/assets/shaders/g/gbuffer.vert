#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec3 outTangent;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	gl_Position = c.proj * c.view * m.model * inPos;
	
	// Vertex position in world space
	outWorldPos = vec3(m.model * inPos);
	// GL to Vulkan coord space
	outWorldPos.y = -outWorldPos.y;
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(m.model)));
	outNormal = mNormal * normalize(inNormal);	
	
	// Currently just vertex color
	outColor = inColor;
}
