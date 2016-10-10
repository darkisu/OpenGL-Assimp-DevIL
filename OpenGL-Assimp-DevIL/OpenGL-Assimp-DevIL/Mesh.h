#pragma once

// Custom Classes Includes
#include "Shader.h"
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;
// GL Includes
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// Assimp Includes
#include <assimp\Importer.hpp>

struct Vertex {
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
};

struct Material {
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
};


struct Texture {
	GLuint id;
	string type;
	aiString path;
};


class Mesh
{
private:
	GLuint VAO, VBO, EBO;

	void setupMesh();

public:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	Material material;

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, Material material);
	~Mesh();

	void Draw(Shader shader);

};

