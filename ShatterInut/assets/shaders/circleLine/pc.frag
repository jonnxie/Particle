#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in float inWidth;

layout (location = 0) out vec4 outFragColor;

void main ()
{
    vec2 coor = gl_PointCoord - vec2(0.5f,0.5f);
    float radius = length(coor);
    if(radius > 0.5f || radius < (0.5 - inWidth))
    {
        discard;
    }
    outFragColor = vec4(inColor,1.0);
}