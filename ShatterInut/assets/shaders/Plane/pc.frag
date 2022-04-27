#version 450

layout(set = 2,binding = 0) uniform UniformColorObject{
    vec3 color;
}c;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 256.0f;

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main() {
    outPosition = vec4(inWorldPos, 1.0);

    vec3 N = normalize(inNormal);
    N.y = -N.y;
    outNormal = vec4(N, 1.0);
    outAlbedo.rgb = c.color;
    // Store linearized depth in alpha component
    outPosition.a = linearDepth(gl_FragCoord.z);

    outColor = vec4(0.0);
}
