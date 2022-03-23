#version 450

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main ()
{
    vec2 coor = gl_PointCoord - vec2(0.5f,0.5f);
    if(length(coor) > 0.5f)
    {
        discard;
    }
    outFragColor = vec4(inColor,1.0);
}