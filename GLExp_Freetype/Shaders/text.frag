#version 450 core

in vec2 vUV;
flat in int vLayer;

out vec4 FragColor;

uniform sampler2DArray textAtlas; // массив текстур
uniform vec3 textColor;

void main()
{
    // читаем красный канал текстуры (альфа)
    float alpha = texture(textAtlas, vec3(vUV, vLayer)).r;

    // комбинируем с цветом текста
    FragColor = vec4(textColor, alpha);

    // можно добавить premultiplied alpha, если надо
}
