#version 450 core 

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldpos;
layout(location = 2) out vec4 normals;

in vec3 fragPos;
in vec2 fragTex;
in vec3 fragNor;
in vec4 fragViewPos;

uniform vec3 campos;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex2;


void main()
{
	vec3 normal = normalize(fragNor);
	vec3 texturecolor = texture(tex, fragTex).rgb;
	
	color = vec4(texturecolor, 1.0);
	worldpos = vec4(fragPos, 1.0);
	normals = vec4(normal, 1.0);
	//viewpos.z*=-1;


}
