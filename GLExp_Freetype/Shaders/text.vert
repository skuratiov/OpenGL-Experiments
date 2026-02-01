#version 450 core

layout(location = 0) in vec2 inPos; // координаты квадрата: 0..1

uniform mat4 uProjection;

// uniform массивы с данными каждого символа
uniform vec2 glyphPos[255];
uniform vec2 glyphSize[255];
uniform vec4 glyphUV[255];
uniform int  glyphLayer[255];

out vec2 vUV;
flat out int vLayer;

void main()
{
    // Берём данные для текущего инстанса
    int id = gl_InstanceID;

    vec2 pos  = glyphPos[id];
    vec2 size = glyphSize[id];
    vec4 uv   = glyphUV[id];

    // Вычисляем позицию вертекса на GPU
    vec2 vertexPos = pos + inPos * size;

    gl_Position = uProjection * vec4(vertexPos, 0.0, 1.0);

    // Интерполяция UV для квадрата
    vUV = mix(uv.xy, uv.zw, inPos);

    // слой текстуры
    vLayer = glyphLayer[id];
}
