#version 450 core 

out vec4 color;

in vec2 fragTex;


layout(location = 0) uniform sampler2D tex;
layout(binding = 2,rgba16f) uniform image3D vox_output;

void main()
{
	ivec3 coordpix = ivec3(fragTex.x * 255.0,fragTex.y*255,0.25*255.);
	vec3 voxtexturecolor = imageLoad(vox_output, coordpix).rgb;
	vec3 texturecolor = texture(tex, fragTex).rgb;
	color.rgb = voxtexturecolor + texturecolor;
	color.a = 1.0;
}
