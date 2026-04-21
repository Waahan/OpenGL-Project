#version 330 core

out vec4 FragColor;
in vec3 ourColor;
in vec2 textureCoordinate;

uniform sampler2D ourTexture;

void main()
{
    //FragColor = vec4(0.356862745f, 0.807843137f, 0.980392157f, 1.0f);
    FragColor = texture(ourTexture, textureCoordinate);
}
