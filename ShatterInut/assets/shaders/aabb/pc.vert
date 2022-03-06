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

out gl_PerVertex{
    vec4 gl_Position;
};

void main() {
    gl_Position = c.proj * c.view * m.model * vec4(inPosition, 1.0);
    gl_Position /= gl_Position.w;
}
