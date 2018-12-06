#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 instancePosOffset;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 camoff;
uniform float timeStamp;
uniform vec3 wind;

out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;

float hash(float n) { return fract(sin(n) * 753.5453123); }
float snoise(vec3 x)
	{
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	float n = p.x + p.y * 157.0 + 113.0 * p.z;
	return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
		mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
		mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
			mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
	}
//Changing octaves, frequency and presistance results in a total different landscape.
float noise(vec3 position, int octaves, float frequency, float persistence) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += snoise(position * frequency) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
		}
	return total / maxAmplitude;
	}

vec3 calcTranslation(float t, vec3 wind_scale) {
	return wind_scale * (sin(t) + 0.5);
}

void main()
{
	vertex_normal = vec4(M * vec4(vertNor,0.0)).xyz;
vec4 tpos =  M * vec4(vertPos + instancePosOffset, 1.0);
tpos.z -=camoff.z;
tpos.x -=camoff.x;
//I.e. (vertex shader):
float height = noise(tpos.xzy, 11, 0.03, 0.6);
float baseheight = noise(tpos.xzy, 4, 0.004, 0.3);
baseheight = pow(baseheight, 5)*3;
height = baseheight*height;
height*=60;

	tpos = tpos;


	tpos.y += height;
	vertex_pos = tpos.xyz;


	if (vertTex.y < 0.1) {

		//vec3 vVertexTranslation = calcTranslation(timeStamp, wind);
		vec3 tpos2 = vertex_pos + vec3(1.0, 0.0, 1.0) * (sin(timeStamp *2) + 0.5);
		vertex_pos = tpos2;
	}
	vertex_tex = vertTex;
	gl_Position = P * V * vec4(vertex_pos,1.0);

}
