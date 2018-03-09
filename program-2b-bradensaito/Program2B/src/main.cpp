/* Program 2B
	Braden Saito
	Built off of Lab5
*/
#define _USE_MATH_DEFINES

#include <iostream>
#include <glad/glad.h>
#include <math.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"
#include "MatrixStack.h"

// used for helper in perspective
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Matrix
{

public:

	static void printMat(float *A, const char *name = 0)
	{
		// OpenGL uses col-major ordering:
		// [ 0  4  8 12]
		// [ 1  5  9 13]
		// [ 2  6 10 14]
		// [ 3  7 11 15]
		// The (i, j)th element is A[i+4*j].

		if (name)
		{
			printf("%s=[\n", name);
		}

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				printf("%- 5.2f ", A[i + 4 * j]);
			}
			printf("\n");
		}

		if (name)
		{
			printf("];");
		}
		printf("\n");
	}

	static void createIdentityMat(float *M)
	{
		// set all values to zero
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				M[i + 4 * j] = 0;
			}
		}

		// overwrite diagonal with 1s
		M[0] = M[5] = M[10] = M[15] = 1;
	}

	static void createTranslateMat(float *T, float x, float y, float z)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				T[i + 4 * j] = 0;
			}
		}
		T[0] = T[5] = T[10] = T[15] = 1;
		T[12] = x;
		T[13] = y;
		T[14] = z;
	}

	static void createScaleMat(float *S, float x, float y, float z)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				S[i + 4 * j] = 0;
			}
		}

		S[0] = x;
		S[5] = y;
		S[10] = z;
		S[15] = 1;
	}

	static void createRotateMatX(float *R, float radians)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				R[i + 4 * j] = 0;
			}
		}

		R[0] = R[15] = 1;

		R[5] = cos(radians);
		R[6] = sin(radians);
		R[9] = -sin(radians);
		R[10] = cos(radians);
	}

	static void createRotateMatY(float *R, float radians)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				R[i + 4 * j] = 0;
			}
		}

		R[5] = R[15] = 1;

		R[0] = cos(radians);
		R[2] = -sin(radians);
		R[8] = sin(radians);
		R[10] = cos(radians);
	}

	static void createRotateMatZ(float *R, float radians)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				R[i + 4 * j] = 0;
			}
		}

		R[0] = cos(radians);
		R[1] = sin(radians);
		R[4] = -sin(radians);
		R[5] = cos(radians);
		R[10] = R[15] = 1;
	}

	static void multMat(float *C, const float *A, const float *B)
	{
		float c = 0;

		for (int k = 0; k < 4; ++k)
		{
			// Process kth column of C
			for (int i = 0; i < 4; ++i)
			{
				// Process ith row of C.
				// The (i,k)th element of C is the dot product
				// of the ith row of A and kth col of B.
				c = 0;

				// vector dot product
				for (int j = 0; j < 4; ++j)
				{
					c += A[i + 4 * j] * B[j + 4 * k];
				}


				C[i + 4 * k] = c;
			}
		}
	}

	static void createPerspectiveMat(float *m, float fovy, float aspect, float zNear, float zFar)
	{
		// http://www-01.ibm.com/support/knowledgecenter/ssw_aix_61/com.ibm.aix.opengl/doc/openglrf/gluPerspective.htm%23b5c8872587rree
		float f = 1.0f / glm::tan(0.5f * fovy);

		m[0] = f / aspect;
		m[1] = 0.0f;
		m[2] = 0.0f;
		m[3] = 0.0f;
		m[4] = 0;

		m[5] = f;
		m[6] = 0.0f;
		m[7] = 0.0f;
		m[8] = 0.0f;

		m[9] = 0.0f;
		m[10] = (zFar + zNear) / (zNear - zFar);
		m[11] = -1.0f;
		m[12] = 0.0f;

		m[13] = 0.0f;
		m[14] = 2.0f * zFar * zNear / (zNear - zFar);
		m[15] = 0.0f;
	}

};

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> tprog;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;
	GLuint IndexBufferID;
	GLuint VertexBuffer2ID;

	float rotation = 0.0f;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			rotation -= 0.2;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			rotation += 0.2;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
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
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		// Initialize the GLSL program.
		tprog = make_shared<Program>();
		tprog->setVerbose(true);
		tprog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/transparent_frag33.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		tprog->init();
		tprog->addUniform("P");
		tprog->addUniform("MV");
		tprog->addAttribute("vertPos");
		tprog->addAttribute("vertNor");
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();


		//VAO and stuff for cylinder
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		float radius = 0.7f;

		//(2pi/80) * i

		GLfloat g_vertex_buffer_data[243] = { 0 };

		float height = 4.0f;

		for (int i = 0; i < 120; i += 3)
		{
			g_vertex_buffer_data[i] = radius * cos(((M_PI*2.0f) / 40.0f) * (i / 3));
			g_vertex_buffer_data[i + 1] = radius * sin(((M_PI*2.0f) / 40.0f) * (i / 3));
			g_vertex_buffer_data[i + 2] = height;
			//cout << g_vertex_buffer_data[i] << " " << g_vertex_buffer_data[i + 1] << " " << g_vertex_buffer_data[i + 2] << endl;
		}

		height = 0.0f;

		for (int i = 120; i < 240; i+=3)
		{
			g_vertex_buffer_data[i] = radius * cos(((M_PI*2.0f) / 40.0f) * ((i -120) / 3));
			g_vertex_buffer_data[i + 1] = radius * sin(((M_PI*2.0f) / 40.0f) * ((i - 120) / 3));
			g_vertex_buffer_data[i + 2] = height;
			//cout << g_vertex_buffer_data[i] << " " << g_vertex_buffer_data[i + 1] << " " << g_vertex_buffer_data[i + 2] << endl;
		}

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//trying this out
		glEnableVertexAttribArray(1);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

		GLuint g_index_buffer_data[240] = { 0 };

		int count = 0;

		for (int i = 0; i < 120; i += 3)
		{
			if (i >= 117)
			{
				g_index_buffer_data[i] = count;
				count++;
				g_index_buffer_data[i + 1] = 0;
				g_index_buffer_data[i + 2] = 79;
			}
			else
			{
				g_index_buffer_data[i] = count;
				count++;
				g_index_buffer_data[i + 1] = count;
				g_index_buffer_data[i + 2] = count + 39;
			}
			//cout << g_index_buffer_data[i] << " " << g_index_buffer_data[i + 1] << " " << g_index_buffer_data[i + 2] << endl;
		}
		for (int i = 120; i < 240; i += 3)
		{
			if (i >= 237)
			{
				g_index_buffer_data[i] = count;
				count++;
				g_index_buffer_data[i + 1] = 40;
				g_index_buffer_data[i + 2] = 0;
			}
			else
			{
				g_index_buffer_data[i] = count;
				count++;
				g_index_buffer_data[i + 1] = count;
				g_index_buffer_data[i + 2] = count - 40;
			}

			//cout << g_index_buffer_data[i] << " " << g_index_buffer_data[i + 1] << " " << g_index_buffer_data[i + 2] << endl;
		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), g_index_buffer_data, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}

	void render()
	{
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		//auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();

		float P[16] = { 0 };

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the local matrices for lab 5
		float aspect = width / (float)height;
		Matrix::createPerspectiveMat(P, 70.0f, aspect, 0.1f, 100.0f);

		MV->pushMatrix();
		

		glm::vec3 tr = { 0.0, 1.0, 0 };

		//translate
		MV->translate(glm::vec3(0, 0, -14));
		//rotate
		MV->rotate(rotation, glm::vec3(0, 1, 0));

		//push
		MV->pushMatrix();
		//scale
		MV->scale(glm::vec3(1, 1, 1));
		//cockpit sphere
	
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		shape->draw(prog);

		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//rotate
		MV->rotate(-M_PI/2, glm::vec3(1, 0, 0));
		//scale
		MV->scale(glm::vec3(0.2, 0.2, 0.35));
		//top propeller support cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		MV->translate(glm::vec3(0, 1.5, 0));
		MV->pushMatrix();
		//rotate
		MV->rotate(M_PI  * glfwGetTime(), glm::vec3(0, 1, 0));
		//scale
		MV->scale(glm::vec3(0.2, 0.2, 1));
		//propeller cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		MV->rotate(M_PI  * glfwGetTime(), glm::vec3(0, 1, 0));
		//scale
		MV->scale(glm::vec3(0.2, 0.2, -1));
		//propeller cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		//rotate
		MV->rotate(M_PI /2, glm::vec3(1, 0, 0));
		MV->rotate(M_PI / 5, glm::vec3(0, 1, 0));
		//scale
		MV->scale(glm::vec3(0.2, 0.2, 0.35));
		//left land support cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		MV->translate(glm::vec3(0.85, -1.1, -1.3));
		//rotate
		//scale
		MV->scale(glm::vec3(0.2, 0.2, 0.7));
		//left land cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		//rotate
		MV->rotate(M_PI / 2, glm::vec3(1, 0, 0));
		MV->rotate(-M_PI / 5, glm::vec3(0, 1, 0));
		//scale
		MV->scale(glm::vec3(0.2, 0.2, 0.35));
		//right land support cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		MV->translate(glm::vec3(-0.85, -1.1, -1.3));
		//rotate
		//scale
		MV->scale(glm::vec3(0.2, 0.2, 0.7));
		//right land cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		//rotate
		MV->rotate(M_PI, glm::vec3(1, 0, 0));
		//scale
		MV->scale(glm::vec3(0.2, 0.4, 1.1));
		//back cylinder
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, nullptr);
		//pop
		MV->popMatrix();
		//push
		MV->pushMatrix();
		//translate
		MV->translate(glm::vec3(0, 0, -4.7));
		//rotate
		//scale
		MV->scale(glm::vec3(0.1, 1, 1));
		//back propeller sphere
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		shape->draw(prog);
		MV->popMatrix();



		glBindVertexArray(VertexArrayID);
		glBindVertexArray(0);
		prog->unbind();

		tprog->bind();

		MV->scale(7);
		glUniformMatrix4fv(tprog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(tprog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		shape->draw(tprog);
	}
};

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
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
