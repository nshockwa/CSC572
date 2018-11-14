#version 450

layout(local_size_x = 1, local_size_y = 1) in;   										 //local group of shaders
layout(rgba8, binding = 0) uniform image2D img_blur;   								 //input image
layout (std430, binding=0) volatile buffer shader_data
{
  ivec2 pixels[16384];
};

//pixels[16383].x

void main()
    {
    uint index = uint(gl_GlobalInvocationID.x);
	ivec2 texcoords = pixels[index];
	imageStore(img_blur, texcoords, vec4(0,1,1,1));
    }
