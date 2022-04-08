#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

layout(set = 0,binding = 0) uniform UniformModelObject{
	mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
	mat4 view;
	mat4 proj;
}c;

layout(std430, set = 2, binding = 0) readonly buffer JointMatrices {
	mat4 jointMatrices[];
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec2 outUV;

void main() 
{
	outUV = inUV;

	// Calculate skinned matrix from weights and joint indices of the current vertex
	mat4 skinMat = 
		inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
		inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
		inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
		inJointWeights.w * jointMatrices[int(inJointIndices.w)];

	gl_Position = c.proj * c.view * m.model * skinMat * vec4(inPos.xyz, 1.0);

	// Vertex position in world space
	outWorldPos = vec3(m.model * vec4(inPosition, 1.0));
	// GL to Vulkan coord space
	outWorldPos.y = -outWorldPos.y;

	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(m.model)));
	outNormal = mNormal * normalize(inNormal);

	// Currently just vertex color
	outColor = inColor.rgb;
}