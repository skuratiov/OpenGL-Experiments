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
    // Simple debug coloring: use UV coordinates as color
    vec3 color = vec3(vUV.x, vUV.y, 0.5);
    
    // Alternative: use position as color
    // vec3 color = normalize(abs(vFragPos)) * 0.5 + 0.5;
    
    // Alternative: use normal as color
    // vec3 N = texture(normalMap, vUV).rgb;
    // N = normalize(N * 2.0 - 1.0);
    // N = normalize(vTBN * N);
    // vec3 color = N * 0.5 + 0.5;
    
    // Alternative: use diffuse texture only (no lighting)
    // vec3 color = texture(diffuseMap, vUV).rgb;
    
    FragColor = vec4(color, 1.0);
}