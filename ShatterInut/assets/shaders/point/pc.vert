#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0,binding = 0) uniform UniformModelObject{
    mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
    mat4 view;
    mat4 proj;
}c;

layout(set = 2,binding = 0) uniform UniformViewObject{
    vec2 viewportDim;
}v;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in float inSize;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex{
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    gl_Position = c.proj * c.view * m.model * vec4(inPosition, 1.0);
    vec4 view_pos = vec4(c.view * m.model * vec4(inPosition, 1.0));
    vec4 projectedCorner = c.proj * vec4(0.5f * inSize, 0.5f * inSize, view_pos.z, view_pos.w);

    gl_PointSize = v.viewportDim.x * projectedCorner.x / projectedCorner.w;
    fragColor = inColor;
}
