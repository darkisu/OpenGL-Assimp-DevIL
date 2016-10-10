
// Custom Class Includes
#include "Scene.h"
#include "Camera.h"

// GL Includes
#include <GLEW\glew.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// window initiation
void initiate(int width, int height, string title);

// callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void Do_Movement();


// Window
GLuint screenWidth = 800;
GLuint screenHeight = 600;


// Display Object
GLFWwindow* pWindow;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

// Variables for mouse and camera
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Point of Light
typedef struct
{
	glm::vec3 position;
	glm::vec3 color;
}PointOfLight;


// Point of Light
PointOfLight lightsource;



int main()
{

	initiate( screenWidth, screenHeight,"GLProject Result");
	// Scene ID Variables
	GLuint sponzaID, cubeID;

	// Scenes
	Scene TargetScene(".\\model\\sponza.obj", sponzaID);		// because of assimp design, the file path require '\\' for path separator

	// Shaders
	Shader shader("./shader/defaultshader.glvs", "./shader/defaultshader.glfs", SHADER_FROM_FILE);


	// Point of Light
	lightsource.position = glm::vec3(0.0, 4000.0, 0.0);
	lightsource.color = glm::vec3(1.0, 1.0, 1.0);


	//------Main Loop------//
	while (!glfwWindowShouldClose(pWindow))
	{
		GLfloat currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrame;
		lastFrame = currentFrameTime;

		// Check and call events
		glfwPollEvents();
		Do_Movement();

		// Clear the colorbuffer
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use shader
		shader.Use();

		// View and projection matrix setup
		glViewport(0, 0, screenWidth, screenHeight);
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 1.0f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 VPMatrix = projection * view;
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "VPMatrix"), 1, GL_FALSE, glm::value_ptr(VPMatrix));

		// Setup Light
		glUniform3fv(glGetUniformLocation(shader.Program, "LightPos"), 1, glm::value_ptr(lightsource.position));
		glUniform3fv(glGetUniformLocation(shader.Program, "LightColor"), 1, glm::value_ptr(lightsource.color));

		// Draw using default shader
		TargetScene.Draw(shader);

		glfwSwapBuffers(pWindow);
	}


	return 0;
}

void initiate(int width, int height, string title)
{
	//------GLFW Initiation------//
	if (glfwInit() == false) {
		cerr << "ERROR: GLFW initialization failed! -- (glfwinit)" << endl;
		system("PAUSE");
		glfwTerminate();
	}
	glfwWindowHint(GLFW_SAMPLES, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//------GLFW Window Creating------//
	pWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (pWindow == NULL)
	{
		cerr << "ERROR: Main window creation failed! -- (glfwCreateWindow)" << endl;
		system("PAUSE");
		glfwTerminate();
	}
	glfwMakeContextCurrent(pWindow);

	//------GLFW Options------//
	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(pWindow, key_callback);
	glfwSetCursorPosCallback(pWindow, mouse_callback);
	glfwSetScrollCallback(pWindow, scroll_callback);


	//enable glew for newer version of open GL
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		cerr << "ERROR: GLEW initialization failed! -- (glewInit)" << endl;
		system("PAUSE");
	}

	//------Viewport Definition------//
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void Do_Movement() 
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_LEFT])
		lightsource.position.x -= 100.0f;
	if (keys[GLFW_KEY_RIGHT])
		lightsource.position.x += 100.0f;
	if (keys[GLFW_KEY_UP])
		lightsource.position.z += 100.0f;
	if (keys[GLFW_KEY_DOWN])
		lightsource.position.z -= 100.0f;
}

