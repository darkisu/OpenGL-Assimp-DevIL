#include "Scene.h"
#include <IL\il.h>

GLint TextureFromFile(const char* path, string directory)
{
	//Generate texture ID and load texture data 
	string filename = string(path);
	filename = directory + '\\' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ilInit();

	ILuint ImgId = 0;
	ilGenImages(1, &ImgId);
	ilBindImage(ImgId);
	ILboolean imageFlag = ilLoadImage((ILstring)filename.c_str());

	ILuint width = ilGetInteger(IL_IMAGE_WIDTH);
	ILuint height = ilGetInteger(IL_IMAGE_HEIGHT);
	ILubyte* image = new ILubyte[width * height * 3];
	ilCopyPixels(0, 0, 0, width, height, 1, IL_RGB,
	IL_UNSIGNED_BYTE, image);



	// Assign texture to ID
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	ilBindImage(0);
	ilDeleteImage(ImgId);
	glBindTexture(GL_TEXTURE_2D, 0);
	//SOIL_free_image_data(image);




	return textureID;
}


Scene::~Scene()
{
}

void Scene::Draw(Shader shader)
{


	ModelObject thisModel;

	for (GLuint modelCounter = 0; modelCounter < this->models.size(); modelCounter++)
	{
		if (models[modelCounter].toDraw == false) continue;
		thisModel = models[modelCounter];
		glm::mat4 modelProperties;
		modelProperties = glm::translate(modelProperties, thisModel.translation); // Translate it down a bit so it's at the center of the scene
		modelProperties = glm::scale(modelProperties, thisModel.scale);	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(modelProperties));

		for (GLuint meshCounter = thisModel.startMesh; meshCounter < thisModel.meshCount + thisModel.startMesh; meshCounter++)
		{
			this->meshes[meshCounter].Draw(shader);
		}
	}
}

void Scene::loadModel(string path,GLuint &ID)
{
	// Read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate );
	// Check for errors
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	// Retrieve the directory path of the filepath
	this->directory = path.substr(0, path.find_last_of('\\'));

	// Save the property of this model with which meshes are contained by the model
	GLuint startPoint, meshCount;
	ModelObject thisModel;
	startPoint = this->meshes.size();

	// Process ASSIMP's root node recursively
	this->processNode(scene->mRootNode, scene);
	meshCount = this->meshes.size()-startPoint;

	thisModel.startMesh = startPoint;
	thisModel.meshCount = meshCount;
	thisModel.scale = glm::vec3(1.0, 1.0, 1.0);
	thisModel.translation = glm::vec3(0.0, 0.0, 0.0);
	thisModel.toDraw = true;

	models.push_back(thisModel);

	ID = models.size() - 1;
}






void Scene::processNode(aiNode * node, const aiScene * scene)
{
	// Process each mesh located at the current node
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		// The node object only contains indices to index the actual objects in the scene. 
		// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(this->processMesh(mesh, scene));
	}
	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}

}

Mesh Scene::processMesh(aiMesh * mesh, const aiScene * scene)
{
	// Data to fill
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
						  // Positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;

		vertex.Position = vector;

		// Normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;

		vertex.Normal = vector;

		// Texture Coordinates
		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}
	// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// Process materials
	Material meshMat;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// Diffuse: texture_diffuseN
		// Specular: texture_specularN
		// Normal: texture_normalN

		// 1. Diffuse maps
		vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. Specular maps
		vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		aiColor3D ambient(0.0f, 0.0f, 0.0f);
		material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);

		aiColor3D diffuse(0.0f, 0.0f, 0.0f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

		aiColor3D specular(0.0f, 0.0f, 0.0f);
		material->Get(AI_MATKEY_COLOR_SPECULAR, specular);

		meshMat.Ka.r = ambient.r; meshMat.Ka.g = ambient.g; meshMat.Ka.b = ambient.b;
		meshMat.Kd.r = diffuse.r; meshMat.Kd.g = diffuse.g; meshMat.Kd.b = diffuse.b;
		meshMat.Ks.r = specular.r; meshMat.Ks.g = specular.g; meshMat.Ks.b = specular.b;
	}


	// Return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures, meshMat);
}

vector<Texture> Scene::loadMaterialTextures(aiMaterial * mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		GLboolean skip = false;
		for (GLuint j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == str)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			Texture texture;

			texture.id = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str;
			textures.push_back(texture);
			this->textures_loaded.push_back(texture);  // Add to loaded textures
		}
	}
	return textures;
}
