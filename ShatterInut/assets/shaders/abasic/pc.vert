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
layout(location = 2) in vec2 inCoordiante;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inJoint0;
layout(location = 5) in vec4 inWeight0;
layout(location = 6) in vec4 inTangent;

out gl_PerVertex{
    vec4 gl_Position;
};

void main() {
    gl_Position = c.proj * c.view * m.model * vec4(inPosition, 1.0);
}
