#pragma once

#include "GLCommon.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector> 
#include <fstream>
#include <sstream>

#include "ShaderManager.h"
#include "Camera.h"
#include "Light.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "LightManager.h"

#include "Particle.h"
#include "World.h"
#include "AABBParticleContactGenerator.h"
#include "ParticleParticleContactGenerator.h"
#include "MeshParticleContactGenerator.h"

const float windowWidth = 1200;
const float windowHeight = 640;
bool editMode = true;

ShaderManager gShaderManager;

Camera camera(windowHeight, windowWidth);
float moveSpeed = 1.1f;
float lightMoveSpeed = 0.01f;

int currentEditIndex = 0;

// PROJECT 2 GLOBALS
std::vector<Particle*> particles;
World world(100);

float maxPower = 100.0f;
float startPower = 8.0f;
float currentPower = startPower;
float powerIncrement = 30.0f;

bool shiftPressed = false;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static float getRandom(float low, float high)
{
	return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Movement controls
	if (!editMode)
	{
		if (key == GLFW_KEY_W)
		{
			camera.position += camera.direction * moveSpeed;
		}
		if (key == GLFW_KEY_A)
		{
			// Rotate our camera's direction 90 degrees to the left
			glm::vec3 left;
			constexpr float theta = glm::radians(90.0f);
			left.x = camera.direction.x * cos(theta) + camera.direction.z * sin(theta);
			left.y = 0.0f;
			left.z = -camera.direction.x * sin(theta) + camera.direction.z * cos(theta);
			camera.position += left * moveSpeed;
		}
		if (key == GLFW_KEY_S)
		{
			camera.position -= camera.direction * moveSpeed;
		}
		if (key == GLFW_KEY_D)
		{
			// Rotate our camera's direction 90 degrees to the right
			glm::vec3 right;
			constexpr float theta = glm::radians(-90.0f);
			right.x = camera.direction.x * cos(theta) + camera.direction.z * sin(theta);
			right.y = 0.0f;
			right.z = -camera.direction.x * sin(theta) + camera.direction.z * cos(theta);
			camera.position += right * moveSpeed;
		}
		if (key == GLFW_KEY_SPACE)
		{
			camera.position.y += moveSpeed;
		}
	}

	// Toggle cursor view
	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		editMode = !editMode;
		int cursorOption = editMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, cursorOption);
	}

	if (!shiftPressed && key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
	{
		shiftPressed = true;
		currentPower = startPower;
	}
	else if (shiftPressed && key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
	{
		shiftPressed = false;
		Particle* particle = new Particle(1.0f, camera.position + (camera.direction * 2.0f), 1.0f);
		float radius = getRandom(1.0f, 3.0f);
		particle->SetRadius(radius);
		particle->SetMass(1.0f * radius);
		particle->SetVelocity(camera.direction * std::min(currentPower, maxPower));
		particles.push_back(particle);
		world.AddParticle(particle);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!editMode)
	{
		camera.MoveCamera(xpos, ypos);
	}
}

bool InitializerShaders();

int main(void)
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Initialize our window
	window = glfwCreateWindow(windowWidth, windowHeight, "Midterm", NULL, NULL);

	// Make sure the window initialized properly
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback); // Tell GLFW where our key callbacks are
	glfwSetCursorPosCallback(window, mouse_callback); // Tell GLFW where our mouse callback is

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress); // Give glad this process ID
	glfwSwapInterval(1);

	if (!InitializerShaders())
	{
		return -1;
	}

	// Load textures
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\white_marble.tga";
		TextureManager::GetInstance()->LoadTexture(ss.str(), TextureManager::TextureType::Diffuse, "marble");
	}

	// Load Models
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\ISO_Sphere.ply";
		Model* model = ModelManager::GetInstance()->LoadModel(ss.str(), "lightFrame");
		model->SetWireframe(true);
		model->SetIgnoreLighting(true);
		model->SetIsOverrideColor(true);
		model->SetColorOverride(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\SoccerBall.obj";
		Model* model = ModelManager::GetInstance()->LoadModel(ss.str(), "sphere");
		model->AddTexture(TextureManager::GetInstance()->GetTextureFromFriendlyName("marble"));
	}
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\floor.ply";
		Model* model = ModelManager::GetInstance()->LoadModel(ss.str(), "floor");
		model->AddTexture(TextureManager::GetInstance()->GetTextureFromFriendlyName("marble"));
	}
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\wall1.ply";
		Model* model = ModelManager::GetInstance()->LoadModel(ss.str(), "wall1");
		model->AddTexture(TextureManager::GetInstance()->GetTextureFromFriendlyName("marble"));
	}
	{
		std::stringstream ss;
		ss << SOLUTION_DIR << "Extern\\assets\\models\\wall2.ply";
		Model* model = ModelManager::GetInstance()->LoadModel(ss.str(), "wall2");
		model->AddTexture(TextureManager::GetInstance()->GetTextureFromFriendlyName("marble"));
	}

	CompiledShader shader = *gShaderManager.pGetShaderProgramFromFriendlyName("Shader#1");

	float fpsFrameCount = 0.0f;
	float fpsTimeElapsed = 0.0f;
	float previousTime = static_cast<float>(glfwGetTime());

	LightManager::GetInstance()->AddLight(shader, "l1", glm::vec3(0.0f, 10.0f, 0.0f));
	Light* l1 = LightManager::GetInstance()->GetLight("l1");
	l1->EditLightType(Light::LightType::POINT, 0.0f, 0.0f);
	l1->EditDirection(0.4f, -0.6f, 0.0f, 1.0f);

	LightManager::GetInstance()->AddLight(shader, "l2", glm::vec3(0.0f, 2.0f, 0.0f));
	Light* l2 = LightManager::GetInstance()->GetLight("l2");
	l2->EditLightType(Light::LightType::POINT, 0.0f, 0.0f);
	l2->EditDirection(0.4f, -0.6f, 0.0f, 1.0f);

	// AABB generators (NO LONGER NEEDED)
	//world.AddContactGenerator(new AABBParticleContactGenerator(glm::vec3(-10.0f, -2.0f, -10.0f), glm::vec3(10.0f, -1.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f))); // Floor
	//world.AddContactGenerator(new AABBParticleContactGenerator(glm::vec3(-10.0f, -2.0f, -11.0f), glm::vec3(10.0f, 20.0f, -10.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	//world.AddContactGenerator(new AABBParticleContactGenerator(glm::vec3(8.0f, -1.0f, -10.0f), glm::vec3(10.0f, 20.0f, 10.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
	//world.AddContactGenerator(new AABBParticleContactGenerator(glm::vec3(-10.0f, -1.0f, 8.0f), glm::vec3(10.0f, 20.0f, 10.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	//world.AddContactGenerator(new AABBParticleContactGenerator(glm::vec3(-10.0f, -1.0f, -11.0f), glm::vec3(-10.0f, 20.0f, 10.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

	// PROJECT 2 SETUP 
	// NOTE: IT IS IMPORTANT THAT THE CORRECT TRANSLATION VALUES ARE PROVIDED TO THE MESH GENERATOR
	world.AddContactGenerator(new MeshParticleContactGenerator(ModelManager::GetInstance()->GetModel("floor"), glm::vec3(0.0f, -5.0f, 0.0f)));
	world.AddContactGenerator(new MeshParticleContactGenerator(ModelManager::GetInstance()->GetModel("wall1"), glm::vec3(0.0f, 10.0f, 15.0f)));
	world.AddContactGenerator(new MeshParticleContactGenerator(ModelManager::GetInstance()->GetModel("wall1"), glm::vec3(0.0f, 10.0f, -15.0f)));
	world.AddContactGenerator(new MeshParticleContactGenerator(ModelManager::GetInstance()->GetModel("wall2"), glm::vec3(15.0f, 10.0f, 0.0f)));
	world.AddContactGenerator(new MeshParticleContactGenerator(ModelManager::GetInstance()->GetModel("wall2"), glm::vec3(-15.0f, 10.0f, 0.0f)));
	world.AddContactGenerator(new ParticleParticleContactGenerator());

	// Our actual render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - previousTime;
		previousTime = currentTime;

		// FPS TITLE
		{
			fpsTimeElapsed += deltaTime;
			fpsFrameCount += 1.0f;
			if (fpsTimeElapsed >= 0.03f)
			{
				std::string fps = std::to_string(fpsFrameCount / fpsTimeElapsed);
				std::string ms = std::to_string(1000.f * fpsTimeElapsed / fpsFrameCount);
				std::string newTitle = "FPS: " + fps + "   MS: " + ms;
				glfwSetWindowTitle(window, newTitle.c_str());

	
				fpsTimeElapsed = 0.f;
				fpsFrameCount = 0.f;
			}
		}

		float ratio;
		int width, height;
		glm::mat4 projection;
		glm::mat4 view;

		glfwGetFramebufferSize(window, &width, &height); // Assign width and height to our window width and height
		ratio = width / (float)height;

		glEnable(GL_DEPTH);
		glEnable(GL_DEPTH_TEST); // Enables the Depth Buffer, which decides which pixels will be drawn based on their depth (AKA don't draw pixels that are behind other pixels)

		glViewport(0, 0, width, height); // Specifies the transformation of device coords to window coords 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the buffers

		view = camera.GetViewMatrix();
		projection = glm::perspective(0.6f, ratio, 0.1f, 1000.0f);

		shader.Bind();
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "matView"), 1, GL_FALSE, glm::value_ptr(view)); // Assign new view matrix
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "matProjection"), 1, GL_FALSE, glm::value_ptr(projection)); // Assign projection
		glUniform4f(glGetUniformLocation(shader.ID, "cameraPosition"), camera.position.x, camera.position.y, camera.position.z, 1.0f);

		// Safety, mostly for first frame
		if (deltaTime == 0.0f)
		{
			deltaTime = 0.03f;
		}

		if (shiftPressed)
		{
			currentPower += powerIncrement * deltaTime;
		}

		world.Update(deltaTime);

		// Render the models
		for (Particle* particle : particles)
		{
			float scale = 0.41f * particle->GetRadius();
			ModelManager::GetInstance()->Draw("sphere", shader, particle->GetPosition(), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(scale, scale, scale));
		}

		// Draw walls
		ModelManager::GetInstance()->Draw("floor", shader, glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		ModelManager::GetInstance()->Draw("wall1", shader, glm::vec3(0.0f, 10.0f, 15.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		ModelManager::GetInstance()->Draw("wall1", shader, glm::vec3(0.0f, 10.0f, -15.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		ModelManager::GetInstance()->Draw("wall2", shader, glm::vec3(15.0f, 10.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		ModelManager::GetInstance()->Draw("wall2", shader, glm::vec3(-15.0f, 10.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		// Clean up invalid particles
		std::vector<Particle*>::iterator it;
		for (it = particles.begin(); it != particles.end();)
		{
			if ((*it)->IsValid())
			{
				it++;
			}
			else
			{
				std::vector<Particle*>::iterator removeIt = it;
				Particle* particle = *it;
				it = particles.erase(removeIt);
				world.RemoveParticle(particle);
				delete particle;
			}
		}

		// Show what we've drawn
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	for (Particle* particle : particles)
	{
		world.RemoveParticle(particle);
		delete particle;
	}

	TextureManager::GetInstance()->CleanUp();
	delete TextureManager::GetInstance();

	ModelManager::GetInstance()->CleanUp();
	delete ModelManager::GetInstance();

	glfwDestroyWindow(window); // Clean up the window

	glfwTerminate(); 
	exit(EXIT_SUCCESS);
}


bool InitializerShaders()
{
	std::stringstream ss;

	// "Normal" Shader
	Shader vertexShader;
	ss << SOLUTION_DIR << "Extern\\assets\\shaders\\vertexShader.glsl";
	vertexShader.fileName = ss.str();
	ss.str("");

	Shader fragmentShader;
	ss << SOLUTION_DIR << "Extern\\assets\\shaders\\fragmentShader.glsl";
	fragmentShader.fileName = ss.str();
	ss.str("");

	bool success = gShaderManager.createProgramFromFile("Shader#1", vertexShader, fragmentShader);
	if (success)
	{
		std::cout << "Shaders compiled OK" << std::endl;
	}
	else
	{
		std::cout << "Error making shaders: " << std::endl;
		std::cout << gShaderManager.getLastError() << std::endl;
		return 1;
	}

	return success;
}