#version 450 core

in vec2 vUV;
flat in int vLayer;

out vec4 FragColor;

uniform sampler2DArray textAtlas; 
uniform vec3 textColor;

void main()
{
    float alpha = texture(textAtlas, vec3(vUV, float(vLayer))).r;

    FragColor = vec4(textColor, alpha);
}
