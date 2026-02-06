#version 450 core

in vec2 vUV;             
out vec4 FragColor;

uniform sampler2D fontAtlas;
uniform vec3 textColor;

void main()
{
    float alpha = texture(fontAtlas, vUV).r;
    if (alpha < 0.01) discard;
    FragColor = vec4(textColor, alpha);
}
