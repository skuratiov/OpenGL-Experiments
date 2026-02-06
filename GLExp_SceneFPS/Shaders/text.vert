#version 450 core

layout(location = 0) in vec2 inPos; // координаты квадрата: 0..1

uniform mat4 uProjection;

layout(std140, binding = 0) uniform GlyphBlock
{
    vec4 glyphRect[255];   // x, y, w, h
    vec4 glyphUV[255];     // u0, v0, u1, v1
    ivec4 glyphLayer[255]; // x = layer
};

out vec2 vUV;
flat out int vLayer;

void main()
{
    int id = gl_InstanceID;

    vec4 rect = glyphRect[id];
    vec4 uv   = glyphUV[id];

    vec2 pos  = rect.xy;
    vec2 size = rect.zw;

    vec2 vertexPos = pos + inPos * size;

    vUV = mix(uv.xy, uv.zw, inPos);

    vLayer = glyphLayer[id].x;

    gl_Position = uProjection * vec4(vertexPos, 0.0, 1.0);
}
