/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include <time.h>
#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;

#define STARCOUNT 1899
class ssbo_data
	{
	public:
		vec4 dataA[STARCOUNT];
		vec4 dataB[STARCOUNT];
	
	};
float frand()
	{
	return (float)rand() / (float)RAND_MAX;
	}

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}



class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
	//texture data
	GLuint Texture;
	
	ssbo_data ssbo_CPUMEM;
	GLuint ssbo_GPU_id;
	GLuint computeProgram;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {}
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {}
	void resizeCallback(GLFWwindow *window, int in_width, int in_height) {}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources";			
		int width, height, channels;
		char filepath[1000];
		//texture 1
		string str = resourceDirectory + "/Blue_Giant.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindImageTexture(0, Texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		glGenerateMipmap(GL_TEXTURE_2D);		
		
		//make an SSBO
		for (int ii = 0; ii < STARCOUNT; ii++)
			{
			ssbo_CPUMEM.dataA[ii] = vec4(rand() % 1000, 0.0, 0.0, 0.0);
			ssbo_CPUMEM.dataB[ii] = vec4(0.0, 0.0, 0.0, 0.0);
			}
		glGenBuffers(1, &ssbo_GPU_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_GPU_id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &ssbo_CPUMEM, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_GPU_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind


	}

	//General OGL initialization - set OGL state here
	void init()
	{
		GLSL::checkVersion();
		//load the compute shader
		std::string ShaderString = readFileAsString("../resources/compute.glsl");
		const char *shader = ShaderString.c_str();
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(computeShader, 1, &shader, nullptr);

		GLint rc;
		CHECKED_GL_CALL(glCompileShader(computeShader));
		CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rc));
		if (!rc)	//error compiling the shader file
			{
			GLSL::printShaderInfoLog(computeShader);
			std::cout << "Error compiling fragment shader " << std::endl;
			exit(1);
			}

		computeProgram = glCreateProgram();
		glAttachShader(computeProgram, computeShader);
		glLinkProgram(computeProgram);
		glUseProgram(computeProgram);
		
		GLuint block_index = 0;
		block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "shader_data");
		GLuint ssbo_binding_point_index = 2;
		glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);

	}
	void compute()
	{
		//print data before compute shader
		cout << endl << endl << "BUFFER BEFORE COMPUTE SHADER" << endl << endl;
		for (int i = 0; i < STARCOUNT; i++)
			cout << "dataA: " << ssbo_CPUMEM.dataA[i].x << ", " << ssbo_CPUMEM.dataA[i].y << ", " << ssbo_CPUMEM.dataA[i].z << ", " << ssbo_CPUMEM.dataA[i].w << "   dataB: " << ssbo_CPUMEM.dataB[i].x << ", " << ssbo_CPUMEM.dataB[i].y << ", " << ssbo_CPUMEM.dataB[i].z << ssbo_CPUMEM.dataB[i].w << endl;


		GLuint block_index = 0;
		block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "shader_data");
		GLuint ssbo_binding_point_index = 0;


		glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_GPU_id);


		glUseProgram(computeProgram);

		bool odd = true;

		for (int n = 0; n < STARCOUNT; n++){
			glUniform1i(glGetUniformLocation(computeProgram, "odd"), odd);
			glUniform1i(glGetUniformLocation(computeProgram, "size"), STARCOUNT);
			glDispatchCompute((GLuint)STARCOUNT, (GLuint)1, 1);				//start compute shader
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			odd = !odd;
		}
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		
		//copy data back to CPU MEM

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_GPU_id);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		int siz = sizeof(ssbo_data);
		memcpy(&ssbo_CPUMEM,p, siz);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		//print data after compute shader
		cout << endl << endl << "BUFFER AFTER COMPUTE SHADER" << endl << endl;
		for (int i = 0; i < STARCOUNT; i++)
			cout << "dataA: " << ssbo_CPUMEM.dataA[i].x << ", " << ssbo_CPUMEM.dataA[i].y << ", " << ssbo_CPUMEM.dataA[i].z << ", " << ssbo_CPUMEM.dataA[i].w << "   dataB: " << ssbo_CPUMEM.dataB[i].x << ", " << ssbo_CPUMEM.dataB[i].y << ", " << ssbo_CPUMEM.dataB[i].z << ssbo_CPUMEM.dataB[i].w << endl;

		}


};
//******************************************************************************************
int main(int argc, char **argv)
{
		Application *application = new Application();
	srand(time(0));

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(32, 32, "Dummy", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGL();

	application->init();
	application->initGeom();
	application->compute();
	
	system("pause");
	return 0;
}
