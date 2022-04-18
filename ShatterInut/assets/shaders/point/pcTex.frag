#version 450

layout (location = 0) in vec3 inColor;

layout(set = 3,binding = 0) uniform sampler2D Texture;

layout (location = 0) out vec4 outFragColor;

void main ()
{
    vec2 coor = gl_PointCoord - vec2(0.5f,0.5f);
    if(length(coor) > 0.5f)
    {
        discard;
    }
    outFragColor = vec4(texture(Texture, gl_PointCoord).rgb, 1.0);
}