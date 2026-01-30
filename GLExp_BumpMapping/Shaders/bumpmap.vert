#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vUV;
out vec3 vFragPos;
out mat3 vTBN;

void main() {
    vUV = uv;
    vFragPos = vec3(model * vec4(pos,1.0));

    mat3 rotMat = mat3(model);
    vec3 N = normalize(rotMat * normal);
    vec3 T = normalize(rotMat * tangent);
    T = normalize(T - dot(T,N)*N);
    vec3 B = cross(N,T);
    vTBN = mat3(T,B,N);

    gl_Position = projection * view * model * vec4(pos,1.0);
}
