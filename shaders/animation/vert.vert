#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0,binding = 0) uniform UniformModelObject{
    mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
    mat4 view;
    mat4 proj;
}c;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 inTexCoor;

layout (location = 0) out vec2 outTexCoor;

out gl_PerVertex {
	vec4 gl_Position;
};
void main() {
    gl_Position = c.proj * c.view * m.model * vec4(pos,1.0);
    outTexCoor = inTexCoor;
}
