#version 450 core

in vec2 vUV;
in vec3 vFragPos;
in mat3 vTBN;

layout(std140, binding = 0) uniform U { uvec2 th; };

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform vec3 lightPos;

out vec4 FragColor;

void main() {
    vec3 N = texture(normalMap, vUV).rgb;
    N = normalize(N * 2.0 - 1.0);  // [0,1] -> [-1,1]

    N = normalize(vTBN * N);

    vec3 L = normalize(lightPos - vFragPos);

    float diff = max(dot(N, L), 0.0);

    vec3 color = texture(diffuseMap, vUV).rgb * diff;

    FragColor = vec4(color, 1.0);
}
