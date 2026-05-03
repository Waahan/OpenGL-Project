#version 330 core

out vec4 FragColor;
in vec3 ourColor;
in vec2 textureCoordinate;

uniform sampler2D ourTexture;
uniform sampler2D otherTexture;

void main()
{
    //FragColor = vec4(0.356862745f, 0.807843137f, 0.980392157f, 1.0f);
    FragColor = mix(texture(ourTexture, textureCoordinate), texture(otherTexture, textureCoordinate), 0.2);

    /*FragColor = mix(texture(ourTexture, textureCoordinate), texture(otherTexture, textureCoordinate), 0.2);
    float grey = dot(FragColor.rgb, vec3(0.299, 0.587, 0.114));
    float side = step(1920 / 2, gl_FragCoord.x);
    FragColor = mix(FragColor, vec4(grey, grey, grey, 1.0), side);*/

    /*FragColor = mix(texture(ourTexture, textureCoordinate), texture(otherTexture, textureCoordinate), 0.2);
    float grey = dot(FragColor.rgb, vec3(0.299, 0.587, 0.114));
    float edge = 100.0;
    float side = smoothstep((1920 / 2) - edge, (1920 / 2) + edge, gl_FragCoord.x);
    FragColor = mix(FragColor, vec4(grey, grey, grey, 1.0), side);*/
}
