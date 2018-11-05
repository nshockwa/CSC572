/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt
*/
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;






class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> geoProg, lightProg, ssaoProg, blurProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape,sponza;
	
	//camera
	camera mycam;
	camera lightcam;

	struct Light {
		vec3 Position;
		vec3 Color;

		float Linear;
		float Quadratic;
		float Radius;
	};


	//texture for sim
	GLuint TextureEarth;
	GLuint TextureMoon,FBOtex,fb,depth_rb, FBOpos, FBOnor, noiseTexture, ssaoFBO, ssaoColorBuffer, ssaoBlurFBO, ssaoColorBufferBlur;

	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;
	float shadeFactor = 1.0;
	GLuint VertexArrayIDBox, VertexBufferIDBox, VertexBufferTex;
	glm::vec3 lightPos = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 lightColor = glm::vec3(0.2 *shadeFactor, 0.2*shadeFactor, 0.7*shadeFactor);
	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	void randomizeColor() {
		lightColor.r = (float)rand() / (float)RAND_MAX;
		lightColor.g = (float)rand() / (float)RAND_MAX;
		lightColor.b = (float)rand() / (float)RAND_MAX;
	}	
	void addShade() {
		lightColor.r = lightColor.r *  0.95;
		lightColor.g = lightColor.g * 0.95;
		lightColor.b = lightColor.b * 0.95;
		
	}	
	void addTint() {
		shadeFactor = 1;
		lightColor.r = (lightColor.r) * 1.05;
		lightColor.g = (lightColor.g) * 1.05;
		lightColor.b = (lightColor.b) * 1.05;
	}

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
		{
			addShade();
		}
		if (key == GLFW_KEY_MINUS && action == GLFW_RELEASE)
		{
			//shadeFactor = 0;
		}


		if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
		{
			addTint();
		}
		if (key == GLFW_KEY_EQUAL && action == GLFW_RELEASE)
		{
			//mycam.w = 0;
		}

		if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			randomizeColor();
		}
		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			//mycam.w = 0;
		}
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}



		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			lightcam.w = 1;
		}
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
		{
			lightcam.w = 0;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			lightcam.s = 1;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		{
			lightcam.s = 0;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			lightcam.a = 1;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		{
			lightcam.a = 0;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			lightcam.d = 1;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		{
			lightcam.d = 0;
		}




	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{


		GLSL::checkVersion();

		
		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//culling:
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		//transparency
		glEnable(GL_BLEND);
		//next function defines how to mix the background color with the transparent pixel in the foreground. 
		//This is the standard:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

		// Initialize the GLSL program.
		geoProg = make_shared<Program>();
		geoProg->setVerbose(true);
		geoProg->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (! geoProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		geoProg->init();
		geoProg->addUniform("P");
		geoProg->addUniform("V");
		geoProg->addUniform("M");
		geoProg->addUniform("campos");
		geoProg->addAttribute("vertPos");
		geoProg->addAttribute("vertNor");
		geoProg->addAttribute("vertTex");

		ssaoProg = make_shared<Program>();
		ssaoProg->setVerbose(true);
		ssaoProg->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/ssao_shader.glsl");
		if (!ssaoProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		ssaoProg->init();
		ssaoProg->addUniform("P");
		ssaoProg->addUniform("V");
		ssaoProg->addUniform("M");
		ssaoProg->addUniform("samples");
		ssaoProg->addAttribute("vertPos");
		ssaoProg->addAttribute("vertTex");


		blurProg = make_shared<Program>();
		blurProg->setVerbose(true);
		blurProg->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/blur_shader.glsl");
		if (!blurProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		blurProg->init();
		blurProg->addUniform("P");
		blurProg->addUniform("V");
		blurProg->addUniform("M");
		blurProg->addAttribute("vertPos");
		blurProg->addAttribute("vertTex");


		lightProg = make_shared<Program>();
		lightProg->setVerbose(true);
		lightProg->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/frag_nolight.glsl");
		if (!lightProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		lightProg->init();
		lightProg->addUniform("P");
		lightProg->addUniform("V");
		lightProg->addUniform("M");
		lightProg->addUniform("campos");
		lightProg->addAttribute("vertPos");
		lightProg->addAttribute("vertTex");

		//finalProg = make_shared<Program>();
		//finalProg->setVerbose(true);
		//finalProg->setShaderNames(resourceDirectory + "/vert.glsl", resourceDirectory + "/blur_shader.glsl");
		//if (!finalProg->init())
		//{
		//	std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
		//	exit(1);
		//}
		//finalProg->init();

	}
	
	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		//init rectangle mesh (2 triangles) for the post processing
		glGenVertexArrays(1, &VertexArrayIDBox);
		glBindVertexArray(VertexArrayIDBox);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferIDBox);

		for (unsigned int i = 0; i < 64; ++i)
		{
			float x = (float)rand() / (float)RAND_MAX * 2.0 - 1.0;
			float y = (float)rand() / (float)RAND_MAX * 2.0 - 1.0;
			float z = (float)rand() / (float)RAND_MAX;
			glm::vec3 sample(x,y,z);
			sample = glm::normalize(sample);
			sample *= (float)rand() / (float)RAND_MAX;
			float scale = (float)i / 64.0;
			ssaoKernel.push_back(sample);
		}
		
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				((float)rand() / (float)RAND_MAX) * 2.0 - 1.0,
				(float)rand() / (float)RAND_MAX * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		GLfloat *rectangle_vertices = new GLfloat[18];
		// front
		int verccount = 0;

		rectangle_vertices[verccount++] = 0.0, rectangle_vertices[verccount++] = 0.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 0.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;
		rectangle_vertices[verccount++] = 0.0, rectangle_vertices[verccount++] = 1.0, rectangle_vertices[verccount++] = 0.0;


		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), rectangle_vertices, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferTex);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferTex);

		float t = 1. / 100.;
		GLfloat *rectangle_texture_coords = new GLfloat[12];
		int texccount = 0;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 1;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 0;
		rectangle_texture_coords[texccount++] = 1, rectangle_texture_coords[texccount++] = 1;
		rectangle_texture_coords[texccount++] = 0, rectangle_texture_coords[texccount++] = 1;

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), rectangle_texture_coords, GL_STATIC_DRAW);
		//we need to set up the vertex array
		glEnableVertexAttribArray(2);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		// Initialize mesh.
		sponza = make_shared<Shape>();
		string sponzamtl = resourceDirectory + "/sponza/";
		sponza->loadMesh(resourceDirectory + "/sponza/sponza.obj", &sponzamtl, stbi_load);
		sponza->resize();
		sponza->init();


		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();
			
		
		int width, height, channels;
		char filepath[1000];

		//texture earth diffuse
		string str = resourceDirectory + "/earth.jpg";
		strcpy(filepath, str.c_str());		
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureEarth);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureEarth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		//texture moon
		str = resourceDirectory + "/moon.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TextureMoon);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureMoon);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(geoProg->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(geoProg->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(geoProg->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glGenFramebuffers(1, &fb);
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);


		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &FBOtex);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
			

		glGenTextures(1, &FBOpos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		/*
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);
	*/
		glGenTextures(1, &FBOnor);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOnor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, NULL);

		//Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, FBOpos, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, FBOnor, 0);

		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?

		//SSAAAAOOOOO
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);


		//blur ish
		glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
		

		//-------------------------
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?

		glUseProgram(lightProg->pid);
		int Tex1Loc = glGetUniformLocation(lightProg->pid, "gPosition");//tex, tex2... sampler in the fragment shader
		int Tex2Loc = glGetUniformLocation(lightProg->pid, "gNormal");
		int Tex3Loc = glGetUniformLocation(lightProg->pid, "gAlbedo");
		int Tex4Loc = glGetUniformLocation(lightProg->pid, "ssao");

		glUniform1i(Tex1Loc, 0);
		glUniform1i(Tex2Loc, 1);
		glUniform1i(Tex3Loc, 2);	
		glUniform1i(Tex4Loc, 3);


		glUseProgram(ssaoProg->pid);
		int sTex1Loc = glGetUniformLocation(ssaoProg->pid, "gPosition");//tex, tex2... sampler in the fragment shader
		int sTex2Loc = glGetUniformLocation(ssaoProg->pid, "gNormal");
		int sTex3Loc = glGetUniformLocation(ssaoProg->pid, "texNoise");
		glUniform1i(sTex1Loc, 0);
		glUniform1i(sTex2Loc, 1);
		glUniform1i(sTex3Loc, 2);

		glUseProgram(blurProg->pid);
		int bTex1Loc = glGetUniformLocation(blurProg->pid, "ssaoInput");//tex, tex2... sampler in the fragment shader
		glUniform1i(bTex1Loc, 0);

		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			cout << "status framebuffer: good";
			break;
		default:
			cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!";
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//*************************************
	double get_last_elapsed_time()
		{
		static double lasttime = glfwGetTime();
		double actualtime = glfwGetTime();
		double difference = actualtime - lasttime;
		lasttime = actualtime;
		return difference;
		}
	//*************************************

	void render_to_texture() // aka render to framebuffer
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, buffers);
		double frametime = get_last_elapsed_time();
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 M, V, S, T, P;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		V = mycam.process();
		//V = glm::mat4(1);


		//bind shader and copy matrices
		geoProg->bind();
		glUniformMatrix4fv(geoProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(geoProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3fv(geoProg->getUniform("campos"), 1, &mycam.pos.x);

		//	******		sponza		******
		float pihalf = 3.1415926 / 2.;
		M = rotate(mat4(1),pihalf,vec3(0,1,0)) * scale(mat4(1), vec3(2.3, 2.3, 2.3));
		glUniformMatrix4fv(geoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		sponza->draw(geoProg, false);	//draw moon
		
		//done, unbind stuff
		geoProg->unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, FBOnor);
		glGenerateMipmap(GL_TEXTURE_2D);
	}


	void render_to_ssao() // aka render to texture
	{
		//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		//GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		//glDrawBuffers(1, buffers);
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		glm::mat4 M, V, S, T, P;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		V = glm::mat4(1);
		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));


		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ssaoProg->bind();
		glUniform3fv(ssaoProg->getUniform("samples"), 64, &ssaoKernel[0].x);
		


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOnor);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		glUniformMatrix4fv(ssaoProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(ssaoProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(ssaoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		ssaoProg->unbind();
	}

	void render_to_blur()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);
		glm::mat4 M, V, S, T, P;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		V = glm::mat4(1);
		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));

		blurProg->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);


		glUniformMatrix4fv(blurProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(blurProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(blurProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		blurProg->unbind();

	}

	void render_to_screen()
	{
		// Get current frame buffer size.

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);
		glm::mat4 M, V, S, T, P;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		V = glm::mat4(1);

		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightProg->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FBOpos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, FBOnor);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		
		glm::vec3 lightPosView = glm::vec3(lightcam.process() * glm::vec4(lightPos, 1.0));
		
		glUseProgram(lightProg->pid);
		GLuint loc = glGetUniformLocation(lightProg->pid, "light.Position");
		GLuint loc2 = glGetUniformLocation(lightProg->pid, "light.Color");
		GLuint loc3 = glGetUniformLocation(lightProg->pid, "light.Linear");
		GLuint loc4 = glGetUniformLocation(lightProg->pid, "light.Quadratic");
		GLuint loc5 = glGetUniformLocation(lightProg->pid, "light.Type");
		glUniform3fv(loc, 1, &lightPosView.x);
		glUniform3fv(loc2, 1, &lightColor.x);
		// Update attenuation parameters
		const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		const float linear = 0.09;
		const float quadratic = 0.032;
		glUniform1f(loc3, linear);
		glUniform1f(loc4, quadratic);

		M = glm::scale(glm::mat4(1), glm::vec3(1.2, 1, 1)) * glm::translate(glm::mat4(1), glm::vec3(-0.5, -0.5, -1));
		glUniformMatrix4fv(lightProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(lightProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(lightProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayIDBox);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		lightProg->unbind();

	}
};
//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(2560, 1440);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render_to_texture();
		application->render_to_ssao();
		application->render_to_blur();
		application->render_to_screen();
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
