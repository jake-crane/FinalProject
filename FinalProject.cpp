// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>

#define M_PI 3.14159265358979323846

using namespace std;

struct Shape {
	GLuint texture;
	vector<vec3> vertices;
	vector<vec2> uvs;
	vector<vec3> normals;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
};

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Final Project", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	GLuint myTextureSamplerUniformLocation  = glGetUniformLocation(programID, "myTextureSampler");

	const char *filePaths[] = {
			"red.bmp", "cube.obj",
			"yellow.bmp", "sphere.obj",
			"grass.bmp", "plane.obj",
			"minecraft_texture.bmp", "minecraft_cube.obj",
			"soccerball.bmp", "soccerball.obj",
			"cone.bmp", "cone.obj",
			"banshee.bmp", "banshee.obj",
			"spyro.bmp", "spyro.obj",
			"di.bmp", "di.obj",
			"MasterChief.bmp", "MasterChief.obj",
			"crash.bmp", "crash.obj",
	};

	const int shapeCount = sizeof(filePaths) / sizeof(char*) / 2;
	Shape shapes[shapeCount];

	int fileIndex = 0;
	for (int i = 0; i < shapeCount; i++, fileIndex += 2) {
		shapes[i].texture = loadBMP_custom(filePaths[fileIndex]);
		if (!loadOBJ(filePaths[fileIndex + 1], shapes[i].vertices, shapes[i].uvs, shapes[i].normals)) {
			perror("Error loading obj file");
			fprintf(stderr, "%s\n", filePaths[fileIndex + 1]);
			return 1;
		}
	}

	for (int i = 0; i < shapeCount; ++i) {
		glGenBuffers(1, &shapes[i].vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, shapes[i].vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].vertices.size() * sizeof(glm::vec3), &shapes[i].vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &shapes[i].uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, shapes[i].uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].uvs.size() * sizeof(glm::vec2), &shapes[i].uvs[0], GL_STATIC_DRAW);

		glGenBuffers(1, &shapes[i].normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, shapes[i].normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].normals.size() * sizeof(glm::vec3), &shapes[i].normals[0], GL_STATIC_DRAW);
	}

	glUseProgram(programID);
	GLuint LightID1 = glGetUniformLocation(programID, "LightPosition1_worldspace");

	glUseProgram(programID);
	GLuint LightID2 = glGetUniformLocation(programID, "LightPosition2_worldspace");

	double lastTime = glfwGetTime() - 10;

	//yellow sphere orbit variables
	const float velocity = M_PI / 144;
	const int radius = 10;
	const float originx = 0, originy = 0, originz = 0;
	float angle_of_rotation = 0.0f;

	vector<vec3> yellow_sphere_origin_verticues(shapes[1].vertices.size());
	for (unsigned int j = 0; j < shapes[1].vertices.size(); j++) {
		yellow_sphere_origin_verticues[j].x = shapes[1].vertices[j].x - 3;
		yellow_sphere_origin_verticues[j].y = shapes[1].vertices[j].y + 1;
		yellow_sphere_origin_verticues[j].z = shapes[1].vertices[j].z - 1;
	}

	glm::vec3 light2Pos = glm::vec3(0, 1.0f, 0);

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		double currentTime = glfwGetTime();

		if (currentTime - lastTime >= .01) {
			angle_of_rotation += velocity ;
			if (angle_of_rotation >= 2 * M_PI) {
				angle_of_rotation = 0;
			}
			for (unsigned int j = 0; j < shapes[1].vertices.size(); j++) {
				shapes[1].vertices[j].x = yellow_sphere_origin_verticues[j].x + originx + radius * cos(angle_of_rotation);
				shapes[1].vertices[j].z = yellow_sphere_origin_verticues[j].z + originz + radius * sin(angle_of_rotation);
			}
			light2Pos.x = originx + radius * cos(angle_of_rotation);
			light2Pos.z = originz + radius * sin(angle_of_rotation);

			glGenBuffers(1, &shapes[1].vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, shapes[1].vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, shapes[1].vertices.size() * sizeof(glm::vec3), &shapes[1].vertices[0], GL_STATIC_DRAW);

			glGenBuffers(1, &shapes[1].uvbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, shapes[1].uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, shapes[1].uvs.size() * sizeof(glm::vec2), &shapes[1].uvs[0], GL_STATIC_DRAW);

			glGenBuffers(1, &shapes[1].normalbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, shapes[1].normalbuffer);
			glBufferData(GL_ARRAY_BUFFER, shapes[1].normals.size() * sizeof(glm::vec3), &shapes[1].normals[0], GL_STATIC_DRAW);
			lastTime = glfwGetTime();
		}


		glm::vec3 light1Pos = glm::vec3(5, 6, 0);
		glUniform3f(LightID1, light1Pos.x, light1Pos.y, light1Pos.z);

		glUniform3f(LightID2, light2Pos.x, light2Pos.y, light2Pos.z);


		for (int i = 0; i < shapeCount; i++) {

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shapes[i].texture);

			glUniform1i(myTextureSamplerUniformLocation, 0);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, shapes[i].vertexbuffer);
			glVertexAttribPointer(
					0,                  // attribute
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, shapes[i].uvbuffer);
			glVertexAttribPointer(
					1,                                // attribute
					2,                                // size
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
			);

			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, shapes[i].normalbuffer);
			glVertexAttribPointer(
					2,                                // attribute
					3,                                // size
					GL_FLOAT,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
			);


			glDrawArrays(GL_TRIANGLES, 0, shapes[i].vertices.size() );

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);

		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0 );

	for (int i = 0; i < shapeCount; ++i) {
		glDeleteBuffers(1, &shapes[i].vertexbuffer);
		glDeleteBuffers(1, &shapes[i].uvbuffer);
		glDeleteProgram(programID);
		glDeleteTextures(1, &myTextureSamplerUniformLocation);
	}
	/*glDeleteBuffers(2, &vertexbuffer2);
		glDeleteBuffers(2, &uvbuffer2);
		glDeleteProgram(programID);
		glDeleteTextures(2, &TextureID2);
		glDeleteVertexArrays(2, &VertexArrayID);*/

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

