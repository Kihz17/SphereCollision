#include "ModelManager.h"
#include "VertexInformation.h"
#include "Mesh.h"
#include "SOIL2.H"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h> // Post processing flags

ModelManager* ModelManager::instance = NULL;

ModelManager::ModelManager()
{

}

ModelManager::~ModelManager()
{

}

ModelManager* ModelManager::GetInstance()
{
	if (ModelManager::instance == NULL)
	{
		ModelManager::instance = new ModelManager();
	}

	return instance;
}

void ModelManager::CleanUp()
{
	std::map<std::string, Model*>::iterator it;
	for (it = this->models.begin(); it != this->models.end(); it++)
	{
		delete it->second;
	}

	this->models.clear();
}

void ModelManager::Draw(std::string friendlyName, const CompiledShader& shader, const glm::vec3& position, const glm::vec3& xRot, const glm::vec3& yRot, const glm::vec3& zRot, const glm::vec3& scale)
{
	Model* model = this->GetModel(friendlyName);
	if (model)
	{
		model->Draw(shader, position, xRot, yRot, zRot, scale);
	}
}

Model* ModelManager::LoadModel(std::string path, std::string friendlyName)
{
	if (this->models.find(friendlyName) != this->models.end())
	{
		std::cout << "Tried to load a model with key '" << friendlyName << "' when it already existed!";
		return NULL;
	}

	Assimp::Importer importer;

	unsigned int flags = 0;
	flags |= aiProcess_Triangulate; // Triangulates the faces (AKA: if there are models with faces > 3, it will turn them into triangles for us)
	flags |= aiProcess_JoinIdenticalVertices; // Joins identical vertex data
	flags |= aiProcess_GenSmoothNormals; // Generates smooth normals for all vertices in the mesh

	const aiScene* scene = importer.ReadFile(path, flags);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Failed to load model!" << std::endl;
		return NULL;
	}

	Model* model = new Model();
	model->directory = path.substr(0, path.find_last_of('\\')) + "\\";
	model->fileName = path.substr(path.find_last_of('\\') + 1, path.length());

	this->LoadAssimpNode(model, scene->mRootNode, scene);

	if (model)
	{
		this->models.insert(std::make_pair(friendlyName, model));
		return model;
	}

	return NULL;
}


void ModelManager::LoadAssimpNode(Model* model, aiNode* node, const aiScene* scene)
{
	// Process assimp meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model->meshes.push_back(this->LoadAssimpMesh(model, mesh, scene));
	}

	// Recursivley processes child nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->LoadAssimpNode(model, node->mChildren[i], scene);
	}
}

Model* ModelManager::GetModel(std::string friendlyName)
{
	std::map<std::string, Model*>::iterator it = this->models.find(friendlyName);
	if (it != this->models.end())
	{
		return it->second;
	}

	return NULL;
}

Mesh ModelManager::LoadAssimpMesh(Model* model, aiMesh* mesh, const aiScene* scene)
{
	std::vector<sVertex> vertices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		sVertex vertex;
		vertex.x = mesh->mVertices[i].x;
		vertex.y = mesh->mVertices[i].y;
		vertex.z = mesh->mVertices[i].z;

		vertex.nx = mesh->mNormals[i].x;
		vertex.ny = mesh->mNormals[i].y;
		vertex.nz = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.u0 = mesh->mTextureCoords[0][i].x;
			vertex.v0 = mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	std::vector<sTriangle> faces;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		sTriangle face;
		aiFace assimpFace = mesh->mFaces[i];
		if (assimpFace.mNumIndices != 3)
		{
			std::cout << "Face is not a triangle!" << std::endl;
		}

		for (unsigned int j = 0; j < assimpFace.mNumIndices; j++)
		{
			face.vertIndex[j] = assimpFace.mIndices[j];
		}

		faces.push_back(face);
	}


	std::vector<Texture*> textures;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Sampler names should adhere to the following convention:
		// Diffuse: texure_diffuseN
		// Specular: texture_specularN
		// Normal: texture_normalN
		// Where N = texture numbers

		for (Texture* texture : this->LoadAssimpMaterialTextures(material, aiTextureType_DIFFUSE, TextureManager::TextureType::Diffuse, model->directory))
		{
			textures.push_back(texture);
		}

		for (Texture* texture : this->LoadAssimpMaterialTextures(material, aiTextureType_SPECULAR, TextureManager::TextureType::Specular, model->directory))
		{
			textures.push_back(texture);
		}
	}

	return Mesh(vertices, faces, textures);
}

std::vector<Texture*> ModelManager::LoadAssimpMaterialTextures(aiMaterial* material, aiTextureType type, TextureManager::TextureType textureType, const std::string& rootPath)
{
	std::vector<Texture*> textures;

	for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
	{
		aiString str;
		material->GetTexture(type, i, &str);
		std::string filePath(str.C_Str());
		std::string fileName = filePath.substr(filePath.find_last_of("\\") + 1, filePath.length());

		std::string lookForPath = rootPath + "Textures\\" + fileName; // Look for the texture file in the directory of the model + /Textures/
		Texture* texture = TextureManager::GetInstance()->LoadTexture(lookForPath, textureType, fileName);
		if (texture)
		{
			textures.push_back(texture);
		}
	}

	return textures;
}