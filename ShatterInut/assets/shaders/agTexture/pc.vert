#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0,binding = 0) uniform UniformModelObject{
    mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
    mat4 view;
    mat4 proj;
}c;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) out vec2 outUV;

out gl_PerVertex{
    vec4 gl_Position;
};

void main() {
    gl_Position = c.proj * c.view * m.model * vec4(inPosition, 1.0);

    // Vertex position in world space
    outWorldPos = vec3(m.model * vec4(inPosition, 1.0));
    // GL to Vulkan coord space
    outWorldPos.y = -outWorldPos.y;

    // Normal in world space
    mat3 mNormal = transpose(inverse(mat3(m.model)));
    outNormal = mNormal * normalize(inNormal);
}
