#version 450 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 vUV;

layout (location = 0) uniform mat4 uProjection;

void main()
{
    vUV = inUV; // correct!
    gl_Position = uProjection * vec4(inPos, 0.0, 1.0);
}
