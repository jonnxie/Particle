#version 450

layout (set = 3, binding = 0) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;


void main() 
{
	vec4 color = texture(samplerColorMap, inUV) * vec4(inColor, 1.0);

	outPosition = vec4(inWorldPos, 1.0);

	vec3 N = normalize(inNormal);
	N.y = -N.y;
	outNormal = vec4(N, 1.0);

	outAlbedo.rgb = color.rgb;

	// Store linearized depth in alpha component
	outPosition.a = linearDepth(gl_FragCoord.z);

	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(0.0);
}