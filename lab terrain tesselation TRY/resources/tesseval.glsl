#version 410 core

layout (triangles, fractional_even_spacing, cw) in;	//equal_spacing
in vec2 TE_vertex_tex[];
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform sampler2D tex;
uniform vec3 camoff;
out vec2 frag_tex;


void main()
{
    vec4 pos = (gl_TessCoord.x * gl_in[0].gl_Position +
                   gl_TessCoord.y * gl_in[1].gl_Position +
                   gl_TessCoord.z * gl_in[2].gl_Position);

	vec2 Tex = (gl_TessCoord.x * TE_vertex_tex[0] +
                   gl_TessCoord.y * TE_vertex_tex[1] +
                   gl_TessCoord.z * TE_vertex_tex[2]);

	
	frag_tex = Tex;
	gl_Position = P * V * pos;

}