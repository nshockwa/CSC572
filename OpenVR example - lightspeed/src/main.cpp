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
#include "OpenVRclass.h"
#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm

#include <time.h>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;

#define STARSCOUNT 10000
#define PI 3.1415926
#define STARTTRANSZ 200
#include "fonts.h"
bmpfont font;
#define GALAXYRADIUS 200.0
#define MAGIC 1.0e8
#include "double_algebra.h"

OpenVRApplication *vrapp = NULL;
float eyeconvergence = 0.1;
float eyedistance = 0.08;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
float frand()
	{
	return (float) rand() / (float) RAND_MAX;
	}

#define LEFTEYE 0
#define RIGHTEYE 1
class camera
{
private:
	vec3 accelangle;
	float accela = 0.003;
	float maxaccela = 1.0;
public:
	long double velocity=0;
	long double lorentz = 1;
	long double maxvel = 0.02;
	double_vec_ pos, rot, viewrot;
	double_vec_ camdir;
	double_mat_ viewM;
	double_mat_ trackingM;
	long double unitsmul_lightyears;
	long double unitsmul_km;
	long double unitsmul_lightsecond;
	int w, a, s, d;
	int key_i, key_j, key_k, key_l;
	camera()
	{
		unitsmul_lightyears = GALAXYRADIUS / 50000.0;
		unitsmul_km = unitsmul_lightyears *9.461e+12;
		unitsmul_lightsecond = unitsmul_lightyears / 3.156e+7;
		maxvel = unitsmul_lightsecond*MAGIC;
		key_i= key_j= key_k= key_l=0;
		w = a = s = d = 0;
		pos = rot = viewrot = double_vec_(0, 0, 0);
		camdir = double_vec_(0, 0, 1);
		viewM.set_identity();
		accelangle = vec3(0, 0, 0);
	
	}
	glm::mat4 process(double ftime,int eye)
	{
		
		long double vdiff = maxvel - velocity;
		if (w == 1)
			{
			velocity += vdiff*0.9 * ftime;
			}
		else if (s == 1)
			{
			velocity /= 1.1;
			}
		else
			velocity -= velocity*0.2 * ftime;

		lorentz = sqrt(1. - pow(velocity, 2) / pow(maxvel, 2));

		
		
		
		float yangle=0;
		if (a == 1)
		{
			if (accelangle.y < 0)accelangle.y = 0;
			accelangle.y = min(accelangle.y, maxaccela);
			accelangle.y += accela * ftime;
			yangle += accelangle.y;
		}
		else if (d == 1)
		{
			if (accelangle.y > 0)accelangle.y = 0;
			accelangle.y = max(accelangle.y, -maxaccela);
			accelangle.y -= accela * ftime;
			yangle += accelangle.y;
		}
		else
			accelangle.y /= 2;
		rot.y += yangle;



		yangle = 0;
		if (key_j == 1)
			yangle = 3 * ftime;
		else if (key_l == 1)
			yangle = -3 * ftime;
		viewrot.y += yangle;
		float xangle = 0;
		if (key_i == 1)
			xangle = 3 * ftime;
		else if (key_k == 1)
			xangle = -3 * ftime;
		viewrot.x += xangle;

		//glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		double_mat_ dR;
		dR.set_rotation_matrix_y(rot.y);
		double_vec_ dir = double_vec_(0, 0, velocity/lorentz);
		dir = dR*dir;
		camdir = dR*double_vec_(0,0,1);
		dir.x *= -1;
		pos += dir;
		double_mat_ T;
		T.set_transform_matrix(pos);	
		double_mat_ M = T*dR;
		mat4 glmM = M.convert_glm();

		
		return glmM;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog,pobj, progFBO, pcircle;
	GLuint VertexBufferID;
	// Contains vertex information for OpenGL
	GLuint VertexArrayID;
	GLuint vaobox,vboboxpos,ibobox;
	// Data necessary to give our box to OpenGL
	
	GLuint vaocircle,vbocicle;
	//texture data
	GLuint Texture;
	GLuint Texture2;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
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
		if (key == GLFW_KEY_I && action == GLFW_PRESS)		mycam.key_i = 1;
		if (key == GLFW_KEY_I && action == GLFW_RELEASE)	mycam.key_i = 0;
		if (key == GLFW_KEY_J && action == GLFW_PRESS)		mycam.key_j = 1;
		if (key == GLFW_KEY_J && action == GLFW_RELEASE)	mycam.key_j = 0;
		if (key == GLFW_KEY_K && action == GLFW_PRESS)		mycam.key_k = 1;
		if (key == GLFW_KEY_K && action == GLFW_RELEASE)	mycam.key_k = 0;
		if (key == GLFW_KEY_L && action == GLFW_PRESS)		mycam.key_l = 1;
		if (key == GLFW_KEY_L && action == GLFW_RELEASE)	mycam.key_l = 0;
		if (key == GLFW_KEY_N && action == GLFW_PRESS)		
		{
			eyeconvergence += 0.1;
			cout << "eyedistance: " << eyedistance << ", eyeconversion: " << eyeconvergence << endl;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) 
		{
			eyeconvergence -= 0.1;
			cout << "eyedistance: " << eyedistance << ", eyeconversion: " << eyeconvergence << endl;
		}
		if (key == GLFW_KEY_B && action == GLFW_PRESS) 
		{ 
			eyedistance += 0.01; 
			cout << "eyedistance: "<< eyedistance << ", eyeconversion: " << eyeconvergence << endl;
		}
		if (key == GLFW_KEY_V && action == GLFW_PRESS)	
		{	
			eyedistance -= 0.01;
			cout << "eyedistance: " << eyedistance << ", eyeconversion: " << eyeconvergence << endl;
		}
		
		

	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
		
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{

		string resourceDirectory = "../resources";
	
		shape = make_shared<Shape>();
		//shape->loadMesh(resourceDirectory + "/t800.obj");
		shape->loadMesh(resourceDirectory + "/box.obj");
		shape->resize();
		shape->init();

		//direction circle:
		glGenVertexArrays(1, &vaocircle);
		glBindVertexArray(vaocircle);
		glGenBuffers(1, &vbocicle);
		glBindBuffer(GL_ARRAY_BUFFER, vbocicle);
		vec3 circle[100];
		float ww = 0;
		float dww = (PI*2.) / 100.0;
		for (int ii = 0; ii < 99; ii++)
		{
			circle[ii] = vec3(sin(ww), cos(ww), 0);
			ww += dww;
		}
		circle[99] = circle[0];
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 100, circle, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//better box:
		glGenVertexArrays(1, &vaobox);
		glBindVertexArray(vaobox);
		glGenBuffers(1, &vboboxpos);
		glBindBuffer(GL_ARRAY_BUFFER, vboboxpos);
		vec3 boxpos[] = {
			vec3(1, 1, -1),
			vec3(-1, 1, -1),
			vec3(-1, -1, -1),
			vec3(1, -1, -1),
			vec3(1, 1, 1),
			vec3(-1, 1, 1),
			vec3(-1, -1, 1),
			vec3(1, -1, 1),
			};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*8, boxpos, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glGenBuffers(1, &ibobox);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibobox);
		unsigned int boxindices[] = {
			0,1,1,2,2,3,3,0,//vorn 
			4,5,5,6,6,7,7,4,//hinten
			0,4,1,5,2,6,3,7//seiten
			};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(unsigned int), boxindices, GL_STATIC_DRAW);


		//generate the VAO STARS
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		vec3 *stars_vertices = new vec3[STARSCOUNT];		
	
		mat4 Rz, Ry, R;
		float w;
		vec4 pos;
		float stretchfact = GALAXYRADIUS;
		for (int i = 0; i < STARSCOUNT; i++)
			{
			float radius = frand();
			radius = pow(radius, 1.5);
			w = frand() * PI - (PI/2.);
			Rz = rotate(mat4(1), w, vec3(0, 0, 1));
			w = frand() * 2.0 * PI;
			Ry = rotate(mat4(1), w, vec3(0, 1, 0));
			R = Ry * Rz;
			pos = vec4(radius*stretchfact, 0, 0, 0);
			pos = R * pos;
			pos.y *= 0.1;
			stars_vertices[i] = vec3(pos);
			}
		


		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*STARSCOUNT, stars_vertices, GL_DYNAMIC_DRAW);
		delete[] stars_vertices;
		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		

	
		glBindVertexArray(0);

	

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		
		GLSL::checkVersion();

		font.init();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.

		//stars shader
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl", resourceDirectory + "/geometry.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addUniform("camdir");
		prog->addUniform("lorentz");
		prog->addUniform("Vdir");
		prog->addAttribute("vertPos");
		//rectangle shader
		pobj = std::make_shared<Program>();
		pobj->setVerbose(true);
		pobj->setShaderNames(resourceDirectory + "/objvertex.glsl", resourceDirectory + "/objfrag.glsl");
		if (!pobj->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
			}
		pobj->addUniform("P");
		pobj->addUniform("V");
		pobj->addUniform("M");
		pobj->addUniform("addcolor");
		pobj->addUniform("campos");
		pobj->addUniform("camdir");
		pobj->addUniform("Vdir");
		pobj->addUniform("lorentz");
		pobj->addAttribute("vertPos");
		pobj->addAttribute("vertNor");
		pobj->addAttribute("vertTex");

		//circle shader
		pcircle = std::make_shared<Program>();
		pcircle->setVerbose(true);
		pcircle->setShaderNames(resourceDirectory + "/circlevertex.glsl", resourceDirectory + "/circlefrag.glsl");
		if (!pcircle->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pcircle->addUniform("P");
		pcircle->addUniform("V");
		pcircle->addUniform("M");		
		pcircle->addUniform("Vdir");	
		pcircle->addAttribute("vertPos");
	

		//FBO shader
		progFBO = std::make_shared<Program>();
		progFBO->setVerbose(true);
		progFBO->setShaderNames(resourceDirectory + "/FBOvertex.glsl", resourceDirectory + "/FBOfragment.glsl");
		if (!progFBO->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progFBO->addUniform("texoff");
		progFBO->addAttribute("vertPos");
		progFBO->addAttribute("vertTex");

		

		
	}
	void render_p(int width, int height,glm::mat4 VRheadmatrix)
	{
		mycam.trackingM = VRheadmatrix;
		static int framecount = 0;
		framecount++;
		double frametime = 0;
		static double totaltime = 0;

		if(framecount%2==1)
			frametime = get_last_elapsed_time();

		totaltime += frametime;
		// Get current frame buffer size.
		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P, Vdir; //View, Model and Perspective matrix
		V = mycam.process(frametime, LEFTEYE);

		Vdir = V;
		//Vdir = mycam.viewM.convert_glm();
		Vdir = VRheadmatrix;
		mat4 Vi = glm::transpose(V);
		Vi[0][3] = 0;
		Vi[1][3] = 0;
		Vi[2][3] = 0;
		//Vdir = transpose(Vdir);
		M = glm::mat4(1);
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 100000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;

		vec3 camdir = mycam.camdir.convertGLM();
		vec3 campos = mycam.pos.convertGLM();


		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -STARTTRANSZ));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransZ * RotateY * RotateX * S;

		// Draw the box using GLSL.
		prog->bind();
		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VertexArrayID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		M = TransZ;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1f(prog->getUniform("lorentz"), mycam.lorentz);
		glUniform3fv(prog->getUniform("campos"), 1, &campos.x);
		glUniform3fv(prog->getUniform("camdir"), 1, &camdir.x);
		glUniformMatrix4fv(prog->getUniform("Vdir"), 1, GL_FALSE, &Vdir[0][0]);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_POINTS, 0, STARSCOUNT);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		prog->unbind();
		// Draw the circle using GLSL.
		pcircle->bind();
		float scaldir = 100.0;
		vec3 pos = vec3(-mycam.pos.x, -mycam.pos.y, -mycam.pos.z) - scaldir * vec3(-mycam.camdir.x, mycam.camdir.y, mycam.camdir.z);
		mat4 Tcstart = translate(mat4(1), pos);
		M = Tcstart;
		glUniformMatrix4fv(pcircle->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pcircle->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pcircle->getUniform("Vdir"), 1, GL_FALSE, &Vdir[0][0]);
		glBindVertexArray(vaocircle);
		glDisable(GL_DEPTH_TEST);
		for (int ii = 0; ii < 4; ii++)
		{
			glm::mat4 Sc = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f*ii, 0.2f*ii, 0.2f*ii));
			mat4 Res = M * Sc*Vi;
			glUniformMatrix4fv(pcircle->getUniform("M"), 1, GL_FALSE, &Res[0][0]);
			glDrawArrays(GL_LINE_STRIP, 0, 100);
		}

		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);
		pcircle->unbind();

		pobj->bind();
		//send the matrices to the shaders
		TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -STARTTRANSZ));
		S = glm::scale(glm::mat4(1.0f), glm::vec3(12.0f, 13.0f, 4 * 10.0f));
		M = TransZ * S;
		//M = mat4(1);
		glUniformMatrix4fv(pobj->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pobj->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pobj->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1f(pobj->getUniform("lorentz"), mycam.lorentz);
		glUniform3fv(pobj->getUniform("campos"), 1, &campos.x);
		glUniform3fv(pobj->getUniform("camdir"), 1, &camdir.x);
		glUniformMatrix4fv(pobj->getUniform("Vdir"), 1, GL_FALSE, &Vdir[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//shape->draw(pobj, FALSE);
		vec3 addcolor;
		addcolor = vec3(1, 1, 0);
		glUniform3fv(pobj->getUniform("addcolor"), 1, &addcolor.x);
		glBindVertexArray(vaobox);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (const void *)0);


		S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		mat4 S2 = glm::scale(glm::mat4(1.0f), glm::vec3(10.5f, 10.5f, 1.0f));
		mat4 Tl;
		addcolor = vec3(1, 1, 1);
		glUniform3fv(pobj->getUniform("addcolor"), 1, &addcolor.x);
		for (int ii = 0; ii < 40; ii++)
		{

			Tl = translate(mat4(1), vec3(0, 0, 100 - 5 * ii));
			if (ii % 2 == 0)
				M = Tl * TransZ*S;
			//	else
			//	M = Tl*TransZ*S2;
			glUniformMatrix4fv(pobj->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (const void *)0);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		pobj->unbind();


		//TEXT
		int seconds = (int)totaltime % 60;
		int minutes = (int)(seconds / 60) % 60;
		int hours = (int)(minutes / 60) % 24;

		float offset_forscreenshots = 0.4;
		long double perc_c = mycam.velocity / mycam.maxvel;
		char text[1000];
		long double f_perc_c = pow(perc_c, 0.01);
		sprintf(text, "speed of light: %.10llf %%", f_perc_c);
		font.draw(-0.4, -0.06 + offset_forscreenshots, text);
		double_vec_ tocam = double_vec_(0, 0, STARTTRANSZ) - mycam.pos;
		long double  absdist = tocam.getlen() / 4.0;
		sprintf(text, "distance to center: %.10llf ", absdist);
		font.draw(-0.4, -0.1 + offset_forscreenshots, text);
		sprintf(text, "time: %d h, %d min, %d s", hours, minutes, seconds);
		font.draw(-0.4, -0.14 + offset_forscreenshots, text);

		long double lorentz = mycam.lorentz;
		//lorentz = pow(lorentz,8);

		static long double galaxytime_sec = 0;
		galaxytime_sec += frametime / lorentz;
		long double lldays = galaxytime_sec / 86400.0;
		long double llyears = lldays / 365.0;
		//	llyears *= 1e10;
		sprintf(text, "galaxy time: %llf y", llyears);
		font.draw(-0.4, -0.18 + offset_forscreenshots, text);

		long double  speed_kmh = perc_c * 1.079e+9;


	}

	
};
Application *application = NULL;
void renderfct(int w, int h, glm::mat4 VRheadmatrix)
{
	application->render_p(w, h, VRheadmatrix);
}
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
	srand(time(0));
	application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();


	vrapp=new OpenVRApplication();

	windowManager->init(vrapp->get_render_width(), vrapp->get_render_height());
	
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();
	vrapp->init_buffers(resourceDir);
	
	

	// Loop until the user closes the window.
	
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{		
		vrapp->render_to_VR(renderfct);
		vrapp->render_to_screen(1);
		
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
