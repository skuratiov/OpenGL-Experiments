#version 450 core

layout (location = 0) in vec3 inPos;   // x, y, layer
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec3 TexCoord;

layout (binding = 0) uniform sampler2DArray textTex;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
};

void main()
{
    gl_Position = projection * vec4(inPos.xy, 0.0, 1.0);
    TexCoord = vec3(inUV, inPos.z);
}
