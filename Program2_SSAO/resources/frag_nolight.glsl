#version 450 core 
out vec4 color;
in vec2 fragTex;
layout(location = 0) uniform sampler2D texcol;
layout(location = 1) uniform sampler2D texpos;
layout(location = 2) uniform sampler2D texnor;
layout(location = 3) uniform sampler2D texssbo;

uniform vec3 campos;

float CosInterpolate(float v1, float v2, float a)
	{
	float angle = a * 3.1415926;
	float prc = (1.0f - cos(angle)) * 0.5f;
	return  v1*(1.0f - prc) + v2*prc;
	}
vec2 calc_depth_fact(vec2 texcoords)
	{
	float depth = texture(texpos, texcoords).b;
	//some number magic:
	float processedDepthFact = depth/7.0;
	processedDepthFact = CosInterpolate(0,5,processedDepthFact);
	processedDepthFact = pow(processedDepthFact,2);
	return vec2(depth,processedDepthFact);
	}

void main()
{
float partx = 1./640.;
float party = 1./480.;
//some extend for a 10 by 10 blurring
float arr[]={0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216,0.001,0.0001,0.00001,0.000001,0.0,0.0};
vec3 texturecolor = texture(texcol, fragTex).rgb;
vec3 texturepos = texture(texpos, fragTex).xyz;
vec3 texturenor = texture(texnor, fragTex).rgb;
vec3 lp = vec3(100,100,100);
vec3 ld = normalize(lp - texturepos);
float light = dot(ld,texturenor);	
light = clamp(light,0,1);

//YEET

//specular light
vec3 camvec = normalize(campos - texturepos);
vec3 h = normalize(camvec+ld);
float spec = pow(dot(h,texturenor),5);
spec = clamp(spec,0,1)*0.3;
	
color.rgb = texturecolor *light + vec3(1,1,1)*spec;

color.a=1;
}
