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

layout(location = 0) out vec3 fragColor;

out gl_PerVertex{
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    gl_Position = c.proj * c.view * m.model * vec4(inPosition, 1.0);
    fragColor = vec3(1.0f,0.64f,0.0f);
}
