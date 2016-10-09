#include "stdafx.h"
#include "Model.h"
#include <thread>


CTextureTA* Model::LoadTexturesForAssimp(const char* pathforfile, std::string directorys)
{
	CTextureTA *temptex = new CTextureTA();
	std::string filenames = std::string(pathforfile);
	filenames = directorys + '\\' + filenames;
	temptex->LoadFreeImage(this->DevClass->GetDevice(),this->DevClass->GetDevCon(),filenames.c_str());

	return temptex;
}

void Model::Release()
{
	for (size_t i = 0; i < meshes.size(); i++)
	{
		meshes[i].Release();
	}
}

/*  Functions   */
// Constructor, expects a filepath to a 3D model.
Model::Model(const char* path, CDeviceClass* devc)
{
	this->DevClass = devc;
	this->loadModel(path);
	this->path_ = path;
}





/*  Functions   */
// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::loadModel(std::string path)
{
	// Read file via ASSIMP
	Assimp::Importer importer;
	
	//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | 
		//aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_CalcTangentSpace );
	const aiScene* scene = importer.ReadFile(path,aiProcess_GenSmoothNormals| aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices|aiProcess_SortByPType);
	// Check for errors
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		MessageBox(NULL, L"scene root node not found", L"ERROR", MB_OK);
		return;
	}
	// Retrieve the directory path of the filepath
	this->directory = path.substr(0, path.find_last_of('\\'));



	// Process ASSIMP's root node recursively
	this->processNode(scene->mRootNode, scene);
	
	importer.FreeScene();
}

XMFLOAT3 Model::XMSubstract(XMFLOAT3 v1, XMFLOAT3 v2)
{
	return XMFLOAT3(v1.x-v2.x,v1.y-v2.y,v1.z-v2.z);
}

XMFLOAT2 Model::XMSubstract(XMFLOAT2 v1, XMFLOAT2 v2)
{
	return XMFLOAT2(v1.x - v2.x, v1.y - v2.y);
}

//Converts 3 component aivector to Xmfloat
XMFLOAT3 Model::aiToXM(aiVector3D v)
{
	return XMFLOAT3(v.x, v.y, v.z);
}


// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Process each mesh located at the current node
	for (uint32 i = 0; i < node->mNumMeshes; i++)
	{
	
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh,scene));
		
	}

	

	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (uint32 i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}

}

XMVECTOR Model::CalculateSurfaceNormal(XMVECTOR p1, XMVECTOR p2, XMVECTOR p3)
{
	XMVECTOR U = p2 - p1;
	XMVECTOR V = p3 - p1;

	XMFLOAT3 Uf;  XMStoreFloat3(&Uf, U);
	XMFLOAT3 Vf;  XMStoreFloat3(&Vf, V);


	return XMVectorSet((Uf.y * Vf.z) - (Uf.z * Vf.y), (Uf.z * Vf.x) - (Uf.x * Vf.z), (Uf.x * Vf.y) - (Uf.y * Vf.x), 1.0f);

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// Data to fill
	std::vector<Vertex> avertices;
	std::vector<uint32> aindices;
	std::vector<Texture> atextures;
	Vertex vertex;

	bool HasNormals = false;
	bool HasTangents = false;

	bool CalcMySelfUV = false;

	// Walk through each of the mesh's vertices
	for (uint32 i = 0; i < mesh->mNumVertices; i++)
	{
		
		XMFLOAT3 vector;// We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// Positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

	


		
		XMFLOAT3 tangent;
		XMFLOAT3 bitangent;
		
			
		if (mesh->HasTangentsAndBitangents())
		{
			//Tangent		
			tangent.x = mesh->mTangents[i].x;
			tangent.y = mesh->mTangents[i].y;
			tangent.z = mesh->mTangents[i].z;
			vertex.Tangent = tangent;

			//bitangent
			bitangent.x = mesh->mBitangents[i].x;
			bitangent.y = mesh->mBitangents[i].y;
			bitangent.z = mesh->mBitangents[i].z;
			vertex.BiTangent = bitangent;
		}
			/*
			else if(i % 3 == 0 && mesh->HasTextureCoords(0))
			{
			

				XMFLOAT3 v0;
				v0.x = mesh->mVertices[i].x;
				v0.y = mesh->mVertices[i].y;
				v0.z = mesh->mVertices[i].z;

				XMFLOAT3 v1;
				v1.x = mesh->mVertices[i + 1].x;
				v1.y = mesh->mVertices[i + 1].y;
				v1.z = mesh->mVertices[i + 1].z;

				XMFLOAT3 v2;
				v2.x = mesh->mVertices[i + 2].x;
				v2.y = mesh->mVertices[i + 2].y;
				v2.z = mesh->mVertices[i + 2].z;

				XMFLOAT2 uv0;
				uv0.x = mesh->mTextureCoords[0][i].x;
				uv0.y = mesh->mTextureCoords[0][i].y;
										
				XMFLOAT2 uv1;			
				uv1.x = mesh->mTextureCoords[0][i + 1].x;
				uv1.y = mesh->mTextureCoords[0][i + 1].y;
	
				XMFLOAT2 uv2;
				uv2.x = mesh->mTextureCoords[0][i + 2].x;
				uv2.y = mesh->mTextureCoords[0][i + 2].y;


				XMFLOAT3 deltaPos1 = this->XMSubstract(v1, v0);
				XMFLOAT3 deltaPos2 = this->XMSubstract(v2, v0);

				XMFLOAT2 deltaUV1 = this->XMSubstract(uv1, uv0);
				XMFLOAT2 deltaUV2 = this->XMSubstract(uv2, uv0);

				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

				XMFLOAT3 deltaPos1UV2;
				deltaPos1UV2.x *= deltaUV2.y;
				deltaPos1UV2.y *= deltaUV2.y;
				deltaPos1UV2.z *= deltaUV2.y;

				XMFLOAT3 deltaPos2UV1;
				deltaPos2UV1.x *= deltaUV1.y;
				deltaPos2UV1.y *= deltaUV1.y;
				deltaPos2UV1.z *= deltaUV1.y;

				tangent = (XMSubstract(deltaPos1UV2, deltaPos2UV1));
				tangent.x *= r;
				tangent.y *= r;
				tangent.z *= r;
				vertex.Tangent = tangent;

				deltaPos2UV1.x *= deltaUV1.x;
				deltaPos2UV1.y *= deltaUV1.x;
				deltaPos2UV1.z *= deltaUV1.x;

				deltaPos1UV2.x *= deltaUV2.x;
				deltaPos1UV2.y *= deltaUV2.x;
				deltaPos1UV2.z *= deltaUV2.x;

				bitangent = XMSubstract(deltaPos2UV1, deltaPos2UV1);
				bitangent.x *= r;
				bitangent.y *= r;
				bitangent.z *= r;
				vertex.BiTangent = bitangent;

			}
		
			
			else
			{
				continue;
			}

	*/
		if (mesh->HasNormals())
		{

		
			// Normals
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		
		}

	
		else
		{
			/*
			//Not working correctly but atleast does not throw an access violation
			XMVECTOR p1 = XMVectorSet(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z,1.0f);
			XMVECTOR p2 = XMVectorSet(mesh->mVertices[i + 1].x, mesh->mVertices[i + 1].y, mesh->mVertices[i + 1].z,1.0f);
			XMVECTOR p3 = XMVectorSet(mesh->mVertices[i + 2].x, mesh->mVertices[i + 2].y, mesh->mVertices[i + 2].z,1.0f);

	

			

			XMFLOAT3 vector;  XMStoreFloat3(&vector,this->CalculateSurfaceNormal(p1, p2, p3));
			vertex.Normal = vector;
			*/
			continue;
		}
	

		// Texture Coordinates
		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			XMFLOAT2 vec;
			// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = XMFLOAT2(0.0f, 0.0f);

		avertices.push_back(vertex);
	}

	// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (uint32 i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];	
		
		// Retrieve all indices of the face and store them in the indices vector
		for (uint32 j = 0; j < face.mNumIndices; j++)
			aindices.push_back(face.mIndices[j]);
	}
	// Process materials
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
		std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		atextures.insert(atextures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. Specular maps
		std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		atextures.insert(atextures.end(), specularMaps.begin(), specularMaps.end());
		// 3. Bump maps
		std::vector<Texture> bumpMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_bump");
		atextures.insert(atextures.end(), bumpMaps.begin(), bumpMaps.end());
		// 3. roughness
		std::vector<Texture> roughnessMaps = this->loadMaterialTextures(material, aiTextureType_SHININESS, "texture_rough");
		atextures.insert(atextures.end(), roughnessMaps.begin(), roughnessMaps.end());
	
	}

	// Return a mesh object created from the extracted mesh data
	return Mesh(avertices, aindices, atextures, DevClass);
}

// Checks all material textures of a given type and loads the textures if they're not loaded yet.
// The required info is returned as a Texture struct.
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (uint32 i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (uint32 j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == str)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			Texture texture;
			texture.tex = LoadTexturesForAssimp(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str;
			textures.push_back(texture);
			this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}




