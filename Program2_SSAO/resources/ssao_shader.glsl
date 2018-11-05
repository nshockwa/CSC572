#version 450 core
out vec4 color;  
in vec2 fragTex;

layout(location = 0) uniform sampler2D gPosition; //must the viewspace!!
layout(location = 1) uniform sampler2D gNormal;
layout(location = 2) uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 P; //that must be the same projection matrix you are using in the render_to_texture ! Don't send it through the rasterizer. 
//rename it to P if need be.
const vec2 noiseScale = vec2(2560.0/4.0, 1440.0/4.0); // screen = 1280x720

void main()
{
vec3 fragPos   = texture(gPosition, fragTex).xyz;//viewpos!!!
vec3 normal    = texture(gNormal, fragTex).rgb;
vec3 randomVec = texture(texNoise, fragTex * noiseScale).xyz;  //using the exact same random vectors from the homepage</strong>
vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal); 

float radius = 0.15;//I have the best results with this values as long as you are using Sponza with my default scaling matrix</strong>
float bias = 0.025;

float occlusion = 0.0;
for(int i = 0; i < 64; ++i)
{    
vec3 smpl = TBN * samples[i]; 
smpl = fragPos + smpl * radius; 
vec4 offset = vec4(smpl, 1.0);
offset      = P * offset;    
offset.xyz /= offset.w;               
offset.xyz  = offset.xyz * 0.5 + 0.5;  
float sampleDepth = texture(gPosition, offset.xy).z; //viewpos!!!
float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
//float rangeCheck = abs(fragPos.z - sampleDepth) < radius ? 1.0 : 0.0;
occlusion       += (sampleDepth >= smpl.z + bias ? 1.0 : 0.0) * rangeCheck;
} 
occlusion = 1.0 - (occlusion / 64);
color = vec4(occlusion,occlusion,occlusion,1); 
//color = vec4(normal,1);
//blur that color later, then finally multiply it with the texturecolor
}