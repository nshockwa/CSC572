#version 330 core
out vec4 color;
in vec2 frag_tex;
uniform sampler2D tex;
uniform sampler2D tex2;
void main()
{
color.rgb = vec3(1);
color.a=1;
}
