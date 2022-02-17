#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140,set = 2, binding = 0) uniform bufferVals {
    float brightVals;
} myBufferVals;
layout (set = 3, binding = 0) uniform sampler2D tex;
layout (location = 0) in vec2 inTexCoor;
layout (location = 0) out vec4 outColor;
void main() {
    outColor=myBufferVals.brightVals*textureLod(tex, inTexCoor, 0.0);
}