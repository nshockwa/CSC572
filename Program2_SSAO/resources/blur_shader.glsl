#version 330 core
out vec4 FragColor;

in vec2 fragTex;

uniform sampler2D ssaoInput;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    vec3 result = vec3(0.0,0.0,0.0);
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += vec3(texture(ssaoInput, fragTex + offset).rgb);
        }
    }
    FragColor = vec4(result / (4.0 * 4.0),1);
}  