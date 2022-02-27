#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2,binding = 0) uniform UniformColorObject{
    vec3 color;
}c;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(c.color, 1.0f);
}
