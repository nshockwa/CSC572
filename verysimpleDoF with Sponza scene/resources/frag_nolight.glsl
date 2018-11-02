#version 450 core 
out vec4 color;
in vec2 fragTex;
layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex2;

float CosInterpolate(float v1, float v2, float a)
	{
	float angle = a * 3.1415926;
	float prc = (1.0f - cos(angle)) * 0.5f;
	return  v1*(1.0f - prc) + v2*prc;
	}
vec2 calc_depth_fact(vec2 texcoords)
	{
	float depth = texture(tex2, texcoords).b;
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
vec3 texturecolor = texture(tex, fragTex).rgb;
vec2 depthfact = calc_depth_fact(fragTex);

vec3 blurcolor = vec3(0,0,0);
for(int x=-10;x<=10;x++)
	for(int y=-10;y<=10;y++)
		{
		if(x==0 && y==0) continue;
		vec2 toff=vec2(partx * x,party * y);
		vec2 distanceVec=vec2(x,y);
		vec3 col = texture(tex, fragTex + toff,0).rgb;
		vec2 depthfact = calc_depth_fact(fragTex + toff);
		float dist = length(distanceVec)-1;
		dist/=4.0;
		dist = clamp(dist,0.0,9.99);
		int idist = int(dist);
		float restdist = dist - idist;
		restdist=0.5;
		float fact = mix(arr[idist],arr[idist+1],restdist)*0.2;
		fact=pow(fact,2)*depthfact.y;
		blurcolor +=col*fact;
		}
	color.rgb = texturecolor*0.8 + blurcolor;
	color.a=1;
	//better results with HDR!
}
