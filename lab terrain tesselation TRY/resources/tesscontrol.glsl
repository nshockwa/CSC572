#version 410 core
layout (vertices = 3) out;

uniform vec3 campos;
in vec3 vertex_pos[];
in vec2 vertex_tex[];
out vec2 TE_vertex_tex[];

void main(void)
{
float tessfact = 1;
 
gl_TessLevelInner[0] = 0;
gl_TessLevelOuter[0] = 1;
gl_TessLevelOuter[1] = 1;
gl_TessLevelOuter[2] = 1;
    
// Everybody copies their input to their output
gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
TE_vertex_tex[gl_InvocationID] = vertex_tex[gl_InvocationID];
}