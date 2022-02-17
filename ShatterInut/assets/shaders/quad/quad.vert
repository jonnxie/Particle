#version 450

layout (location = 0) out vec2 outUV;

void main() 
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) / 4.0f + 0.25f;
	outUV.y = 1 - outUV.y;
//	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
	if(gl_VertexIndex == 0)
	{
		gl_Position = vec4(0.5f, -0.5f, 0.0f, 1.0f);
	}else if(gl_VertexIndex == 1)
	{
		gl_Position = vec4(1.0f, -0.5f, 0.0f, 1.0f);
	}else if(gl_VertexIndex == 2)
	{
		gl_Position = vec4(0.5f, -1.0f, 0.0f, 1.0f);
	}else if(gl_VertexIndex == 3)
	{
		gl_Position = vec4(1.0f, -1.0f, 0.0f, 1.0f);
	}
//	outUV *= 0.5f;
}
