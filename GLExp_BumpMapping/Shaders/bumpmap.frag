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
    const vec3 lightPos = vec3(0.0, 0.0, 4.0);   
    const vec3 viewPos  = vec3(0.0, 0.0, 4.0);   
    const float shininess = 32.0;

    vec3 N = texture(normalMap, vUV).rgb;
    N = normalize(N * 2.0 - 1.0);         // [0,1] -> [-1,1]
    N = normalize(vTBN * N);              // tangent -> world space

    vec3 L = normalize(lightPos - vFragPos);
    float diff = max(dot(N, L), 0.0);

    vec3 V = normalize(viewPos - vFragPos);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N,H), 0.0), shininess);

    vec3 color = texture(diffuseMap, vUV).rgb;
    vec3 finalColor = color * diff + vec3(1.0) * spec;

    FragColor = vec4(finalColor, 1.0);
}

