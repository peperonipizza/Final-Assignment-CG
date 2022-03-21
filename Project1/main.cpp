#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glsl.h"
#include "objloader.h"


using namespace std;


//--------------------------------------------------------------------------------
// Consts
//--------------------------------------------------------------------------------

const int WIDTH = 800, HEIGHT = 600;

const char* fragshader_name = "fragmentshader.fsh";
const char* vertexshader_name = "vertexshader.vsh";

unsigned const int DELTA_TIME = 10;

vector<glm::vec3> normals;
vector<glm::vec3> vertices;
vector<glm::vec2> uvs;


struct LightSource
{
	glm::vec3 position;
};

struct Material {
	glm::vec3 ambient_color;
	glm::vec3 diffuse_color;
	glm::vec3 specular;
	float power;
};

//--------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------

// ID's
GLuint program_id;
GLuint vao;
GLuint vbo_normals;

// Uniform ID's
GLuint uniform_mv;
GLuint normal_id;

// Matrices
glm::mat4 model, view, projection;
glm::mat4 mv;

LightSource light;
Material material;

//--------------------------------------------------------------------------------
// Keyboard handling
//--------------------------------------------------------------------------------

void keyboardHandler(unsigned char key, int a, int b)
{
	if (key == 27)
		glutExit();
}


//--------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------

void Render()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Attach to program_id
	glUseProgram(program_id);

	// Do transformation
	model = glm::rotate(model, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	mv =  view * model;

	// Send mvp
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(mv));

	// Send vao
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);

	glutSwapBuffers();
}


//------------------------------------------------------------
// void Render(int n)
// Render method that is called by the timer function
//------------------------------------------------------------

void Render(int n)
{
	Render();
	glutTimerFunc(DELTA_TIME, Render, 0);
}


//------------------------------------------------------------
// void InitGlutGlew(int argc, char **argv)
// Initializes Glut and Glew
//------------------------------------------------------------

void InitGlutGlew(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Hello OpenGL");
	glutDisplayFunc(Render);
	glutKeyboardFunc(keyboardHandler);
	glutTimerFunc(DELTA_TIME, Render, 0);

	glewInit();
}


//------------------------------------------------------------
// void InitShaders()
// Initializes the fragmentshader and vertexshader
//------------------------------------------------------------

void InitShaders()
{
	char* vertexshader = glsl::readFile(vertexshader_name);
	GLuint vsh_id = glsl::makeVertexShader(vertexshader);

	char* fragshader = glsl::readFile(fragshader_name);
	GLuint fsh_id = glsl::makeFragmentShader(fragshader);

	program_id = glsl::makeShaderProgram(vsh_id, fsh_id);
}


//------------------------------------------------------------
// void InitMatrices()
//------------------------------------------------------------

void InitMatrices()
{
	model = glm::mat4();
	view = glm::lookAt(
		glm::vec3(2.0, 2.0, 7.0),  // eye
		glm::vec3(0.0, 0.0, 0.0),  // center
		glm::vec3(0.0, 1.0, 0.0));  // up
	projection = glm::perspective(
		glm::radians(45.0f),
		1.0f * WIDTH / HEIGHT, 0.1f,
		20.0f);
	mv = view * model;
}

void InitObjects() {
	bool res = loadOBJ("teapot.obj", vertices, uvs, normals);

}

void InitMaterialLight() {
	light.position = glm::vec3(4, 4, 4);
	material.ambient_color = glm::vec3(0.2, 0.2, 0.1);
	material.diffuse_color = glm::vec3(0.5, 0.5, 0.3);
	material.specular = glm::vec3(0.7, 0.7, 0.7);
	material.power = 1024;
}
//------------------------------------------------------------
// void InitBuffers()
// Allocates and fills buffers
//------------------------------------------------------------

void InitBuffers()
{
	GLuint position_id;
	GLuint normal_id;
	GLuint vbo_vertices;
	GLuint vbo_normals;

	// vbo for vertices
	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(glm::vec3), &vertices[0],
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vbo for normals
	glGenBuffers(1, &vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER,
		normals.size() * sizeof(glm::vec3),
		&normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Get vertex attributes
	position_id = glGetAttribLocation(program_id, "position");
	normal_id = glGetAttribLocation(program_id, "normal");

	// Allocate memory for vao
	glGenVertexArrays(1, &vao);

	// Bind to vao
	glBindVertexArray(vao);

	// Bind vertices to vao
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Bind normals to vao
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glVertexAttribPointer(normal_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normal_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Stop bind to vao
	glBindVertexArray(0);

	// Make uniform vars
	uniform_mv = glGetUniformLocation(program_id, "mv");
	GLuint uniform_proj = glGetUniformLocation(program_id, "projection");
	GLuint uniform_light_pos = glGetUniformLocation(program_id, "light_pos");
	GLuint uniform_material_ambient = glGetUniformLocation(program_id,
		"mat_ambient");
	GLuint uniform_material_diffuse = glGetUniformLocation(program_id,
		"mat_diffuse");
	GLuint uniform_specular = glGetUniformLocation(
		program_id, "mat_specular");
	GLuint uniform_material_power = glGetUniformLocation(
		program_id, "mat_power");


	glUseProgram(program_id);
	
	// Fill uniform vars
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(mv));
	glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(uniform_light_pos, 1, glm::value_ptr(light.position));
	glUniform3fv(uniform_material_ambient, 1, glm::value_ptr(material.ambient_color));
	glUniform3fv(uniform_material_diffuse, 1, glm::value_ptr(material.diffuse_color));
	glUniform3fv(uniform_specular, 1, glm::value_ptr(material.specular));
	glUniform1f(uniform_material_power, material.power);

	// Define model
	mv = view * model;

	// Send mvp
	glUseProgram(program_id);
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(mv));

}


int main(int argc, char** argv)
{
	InitGlutGlew(argc, argv);
	InitShaders();
	InitMatrices();
	InitObjects();
	InitMaterialLight();
	InitBuffers();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Hide console window
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);

	// Main loop
	glutMainLoop();

	return 0;
}
