#pragma once

#include "Mesh.h"
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>


GLint TextureFromFile(const char* path, string directory);


typedef struct
{
	GLuint startMesh;
	GLuint meshCount;
	glm::vec3 translation;
	glm::vec3 scale;
	bool toDraw;
}ModelObject;

class Scene
{
public:
	Scene(GLchar* path, GLuint &ID)
	{
		this->loadModel(path, ID);
	}
	~Scene();
	void Draw(Shader shader);
	void loadModel(string path,GLuint &ID);

	void setModel(GLuint ID, bool toDraw)
	{
		models[ID].toDraw = toDraw;
	}
	void setModel(GLuint ID, bool toDraw, glm::vec3 trans, glm::vec3 scal)
	{
		setModel(ID, toDraw);
		models[ID].translation = trans;
		models[ID].scale = scal;
	}


private:
	/*  Model Data  */
	vector<ModelObject> models;
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

										// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.

	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);


};
