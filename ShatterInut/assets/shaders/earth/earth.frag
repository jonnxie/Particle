#version 450

layout(set = 3,binding = 0) uniform sampler2D Texture;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(texture(Texture, vec2(inUV)).rgb,1.0f);
}