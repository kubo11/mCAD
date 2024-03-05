#version 430 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D big_texture;

void main()
{
    FragColor = texture(big_texture, TexCoord);
}