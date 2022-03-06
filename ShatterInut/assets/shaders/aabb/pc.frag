#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2,binding = 0) uniform UniformCameraObject{
    uint captureVal;
}capture;

layout(location = 0) out uint outVal;

void main() {
    outVal = capture.captureVal;
}
