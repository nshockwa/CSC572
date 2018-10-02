#version 450 core 

out vec4 color;

in vec2 fragTex;

uniform int direction;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex2;

void main()
{
	float weights[] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
	float xp = 1. / 480.0;
	vec3 texturecolor = texture(tex, fragTex).rgb;
	texturecolor.r = pow(texturecolor.r, 5);
	texturecolor.g = pow(texturecolor.g, 5);
	texturecolor.b = pow(texturecolor.b, 5);
	for (int i = -10; i < 10; i++)
	{
		vec3 col;
		if (direction == 0){
		col = texture(tex, fragTex + vec2(i*xp, 0), 0).rgb;
		}
		if (direction ==1){
		col = texture(tex, fragTex + vec2(0, i*xp), 0).rgb;
		}
		if (i == 0) continue;
		int wi = abs(i);
		texturecolor += col * weights[wi - 1];
	}
	color.rgb = texturecolor;
	color.a=1;
}
