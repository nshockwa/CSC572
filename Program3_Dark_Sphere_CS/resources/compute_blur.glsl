#version 450

layout(local_size_x = 1, local_size_y = 1) in;   										 //local group of shaders
layout(rgba8, binding = 0) uniform image2D img_blur;   								 //input image
layout (std430, binding=0) volatile buffer shader_data
{
  ivec2 pixels[16384];
};


//void main()
//    {
//    uint index = uint(gl_GlobalInvocationID.x);
//	ivec2 texcoords = pixels[index];
//	   vec2 result[16384];
//
//  int r =2;
//  int n =2;
//  float scale = 1.0 / (2*r + 1); // or use fixed point
//
//  // Compute sum at first pixel. Remember this is an odd-sized
//  // box filter kernel.
//   int sum = texcoords[0].x;
//  for (int i=1; i <= r; i++)
//    sum += texcoords[-i].x + texcoords[i].x;
//
//  // Generate output pixel, then update running sum for next pixel.
//  for (int i=0; i < n; i++) {
//    result[i].y = sum * scale;
//    sum += texcoords[i+r+1].x - texcoords[i-r].x;
//
//  }
//  	imageStore(img_blur, texcoords, vec4(imageLoad(img_blur, texcoords + sum)));
//
//    }


//
void main() 
{
	uint index = uint(gl_GlobalInvocationID.x); 
	if (index> pixels[16383].x) return;
	ivec2 texcoords = ivec2(pixels[index]); 
	vec2 texelSize = vec2(1.0 / 16384);
    vec3 result = vec3(0.0,0.0,0.0);
	int r =5;
    for (int x = -r; x < r; ++x) 
    {
        for (int y = -r; y < r; ++y) 
        {
            ivec2 offset = ivec2(int(x), int(y));
            result += vec3(imageLoad(img_blur, texcoords + offset).rgb);
        }
    }

    vec4 FragColor = vec4(result / (4.0 * 4.0),1);
//	imageStore(img_blur, texcoords, vec4(0.0,1.0,1.0,1.0));
	imageStore(img_blur, texcoords, FragColor);

}
