#version 450 core

layout (location = 0) in vec3 TexCoord;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2DArray textTex;

layout (location = 1) uniform vec3 textColor;

void main()
{
    float alpha = texture(textTex, TexCoord).r;

    FragColor = vec4(textColor, alpha);
}
