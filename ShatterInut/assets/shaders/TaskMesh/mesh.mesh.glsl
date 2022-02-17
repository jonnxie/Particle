#version 450

#extension GL_NV_mesh_shader: require

layout(local_size_x = 1,local_size_y = 1,local_size_z = 1) in;
layout(triangles, max_vertices = 64, max_primitives = 42) out;

layout(set = 0,binding = 0) uniform UniformModelObject{
    mat4 model;
}m;

layout(set = 1,binding = 0) uniform UniformCameraObject{
    mat4 view;
    mat4 proj;
}c;

struct Vertex
{
    vec3 position;
    vec3 color;
};

layout(set = 2,binding = 0) readonly buffer Vertices
{
    Vertex vertices[];
}v;

struct Meshlet
{
    uint vertices[64];
    uint indices[126];
    uint indexCount;
    uint vertexCount;
};

layout(set = 2,binding = 1) readonly buffer Meshlets
{
    Meshlet meshlets[];
}Mesh;

layout(location = 0) out Color{
    vec3 color;
} fragColor[];

void main() {
    uint index = gl_WorkGroupID.x;

    for(uint i = 0; i < uint(Mesh.meshlets[index].vertexCount); ++i)
    {
        uint vi = Mesh.meshlets[index].vertices[i];
        vec3 pos = vec3(v.vertices[vi].position);

        gl_MeshVerticesNV[i].gl_Position = c.proj * c.view * m.model * vec4(pos, 1.0);
        vec3 color = vec3(v.vertices[vi].color);

        fragColor[i].color = color;
    }
    gl_PrimitiveCountNV = uint(Mesh.meshlets[index].indexCount) / 3;

    for(uint i = 0; i < uint(Mesh.meshlets[index].indexCount); ++i)
    {
        gl_PrimitiveIndicesNV[i] = uint(Mesh.meshlets[index].indices[i]);
    }
}
