#version 450 core

layout(location = 0) uniform sampler2D uTexture;

in vec2 vUV;
out vec4 outColor;

void main()
{
    outColor = texture(uTexture, vUV);
}

