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
layout(location = 3) in float inWidth;

layout(location = 0) out vec3 outColor;
layout(location = 1) out float outWidth;

out gl_PerVertex{
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    gl_Position = c.proj * c.view * m.model * vec4(inPosition, 1.0);
    vec4 view_pos = vec4(c.view * m.model * vec4(inPosition, 1.0));
    vec4 projectedCorner = c.proj * vec4(0.5f * inSize, 0.5f * inSize, view_pos.z, view_pos.w);
    vec4 widthProjectedCorner = c.proj * vec4(0.5f * inWidth, 0.5f * inWidth, view_pos.z, view_pos.w);

    outWidth = v.viewportDim.x * widthProjectedCorner.x / widthProjectedCorner.w;
    gl_PointSize = v.viewportDim.x * projectedCorner.x / projectedCorner.w;
    outWidth = outWidth / gl_PointSize;
    outColor = inColor;
}
