/* Program 3
	Braden Saito
	Built off of Lab5/Program2B
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
	std::shared_ptr<Program> prog2;
	std::shared_ptr<Program> progD;
	std::shared_ptr<Program> progS;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;


	float rotation = 0.0f;
	float lightX = -2.0f;

	int mode = 0;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			rotation -= 0.2;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			lightX -= 1.0f;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			lightX += 1.0f;
		}
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
		{
			if (mode == 3)
			{
				mode = 0;
			}
			else
			{
				mode++;
			}
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

		// Initialize the GLSL program (DEFAULT)
		progD = make_shared<Program>();
		progD->setVerbose(true);
		progD->setShaderNames(resourceDirectory + "/simple_vertD.glsl", resourceDirectory + "/simple_fragD.glsl");
		if (!progD->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progD->init();
		progD->addUniform("P");
		progD->addUniform("MV");
		progD->addAttribute("vertPos");
		progD->addAttribute("vertNor");
		//progD->addUniform("light");

		// Initialize the GLSL program (GOURAUD)
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
		prog->addUniform("light");

		// Initialize the GLSL program 2 (PHONG)
		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/simple_vert2.glsl", resourceDirectory + "/simple_frag2.glsl");
		if (!prog2->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("MV");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertNor");
		prog2->addUniform("light");

		// Initialize the GLSL program 4 (SILHOUETTE)
		progS = make_shared<Program>();
		progS->setVerbose(true);
		progS->setShaderNames(resourceDirectory + "/simple_vertS.glsl", resourceDirectory + "/simple_fragS.glsl");
		if (!progS->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		progS->init();
		progS->addUniform("P");
		progS->addUniform("MV");
		progS->addAttribute("vertPos");
		progS->addAttribute("vertNor");
		progS->addUniform("light");

	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory);
		shape->resize();
		shape->init();

	}

	void render()
	{
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		//auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();

		float P[16] = { 0 };

		vec3 light;

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
		MV->translate(glm::vec3(0, 0, -6));
		//rotate
		MV->rotate(rotation, glm::vec3(0, 1, 0));
		//rotation += 0.01f;

		//push
		MV->pushMatrix();

		MV->translate(vec3(-2, 0, 0));

		//scale
		MV->scale(glm::vec3(1, 1, 1));
		//cockpit sphere
	
		if (mode == 0)
		{
			progD->bind();
			glUniformMatrix4fv(progD->getUniform("P"), 1, GL_FALSE, P);
			glUniformMatrix4fv(progD->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		//	glUniform3f(progD->getUniform("light"), lightX, 2, 2);
			shape->draw(progD);
		}

		else if (mode == 1)
		{
			prog->bind();
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(prog->getUniform("light"), lightX, 2, 2);
			shape->draw(prog);
		}

		if (mode == 2)
		{
			prog2->bind();
			glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, P);
			glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(prog2->getUniform("light"), lightX, 2, 2);
			shape->draw(prog2);
		}
		if (mode == 3)
		{
			progS->bind();
			glUniformMatrix4fv(progS->getUniform("P"), 1, GL_FALSE, P);
			glUniformMatrix4fv(progS->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(progS->getUniform("light"), lightX, 2, 2);
			shape->draw(progS);
		}
	//	prog->unbind();

		//pop
		MV->popMatrix();

		MV->pushMatrix();

		MV->translate(vec3(2, 0, 0));

		MV->rotate(1.5, vec3(0, 1, 0));

		//scale
		MV->scale(glm::vec3(1, 1, 1));
		//cockpit sphere

	//	prog2->bind();
		//glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, P);

		if (mode == 0)
		{
			glUniformMatrix4fv(progD->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(progD);

			MV->popMatrix();
			//	prog2->unbind();
			progD->unbind();
		}

		if (mode == 1)
		{
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(prog);

			MV->popMatrix();
			//	prog2->unbind();
			prog->unbind();
		}

		if (mode == 2)
		{
			glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(prog2);

			MV->popMatrix();
			//	prog2->unbind();
			prog2->unbind();
		}
		if (mode == 3)
		{
			glUniformMatrix4fv(progS->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(progS);

			MV->popMatrix();
			//	prog2->unbind();
			progS->unbind();
		}
	}
};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	std::string objectMesh;

	if (argc >= 2)
	{
		objectMesh = argv[1];
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
	application->initGeom(objectMesh);

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
