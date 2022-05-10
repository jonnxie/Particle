#version 450

layout(set = 2,binding = 0) uniform sampler2D Texture;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

void main() {
    outPosition = vec4(inWorldPos, 1.0);

    vec3 N = normalize(inNormal);
    N.y = -N.y;
    outNormal = vec4(N, 1.0);
    outAlbedo.rgb = texture(Texture, inUV).rgb;
    outColor = vec4(0.0);
}
