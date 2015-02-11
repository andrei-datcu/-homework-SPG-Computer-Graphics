#include <glew/glew.h>
#include "Mesh.h"
#include <cstring>
#include <tuple>

unsigned int Mesh::shaderMaterialUniformLocation;
std::unordered_map<std::string, unsigned int> Mesh::texturesId;


Mesh::Mesh(const aiMesh* assimpMesh, const aiScene *assimpScene){

	for (unsigned int faceIndex = 0; faceIndex < assimpMesh->mNumFaces;
		++faceIndex){

		unsigned int *indices = assimpMesh->mFaces[faceIndex].mIndices;
		faces.emplace_back(indices[0], indices[1], indices[2]);
	}

	// generez VAO

	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);

	// IBO
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::uvec3) * faces.size(),
		&(faces[0]), GL_STATIC_DRAW);
	
	if (assimpMesh->HasPositions()) {// VBO Pozitii

		for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
			positions.emplace_back(assimpMesh->mVertices[i].x,
				assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z);

		positions_vbo = generateVBO(positions, 0);
	}

	if (assimpMesh->HasNormals()) {//VBO Normale

		for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
			normals.emplace_back(assimpMesh->mNormals[i].x,
				assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z);
		normals_vbo = generateVBO(normals, 1);
	}

	if (assimpMesh->HasTextureCoords(0)){//VBO coordonate texturi

		for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i)
			texcoords.emplace_back(assimpMesh->mTextureCoords[0][i].x,
			assimpMesh->mTextureCoords[0][i].y);
		texcoords_vbo = generateVBO(texcoords, 2);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	
	// materiale / texturi
	aiMaterial *mtl = assimpScene->mMaterials[assimpMesh->mMaterialIndex];
	aiString texPath;	//potentialul nume de textura

	bool textured = mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) ==
		AI_SUCCESS;

	if(textured)
		textureIndex = texturesId[texPath.data], textureName = texPath.data;
	
	aiColor4D diffuse(0.8f, 0.8f, 0.8f, 1.0f);
	aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse);

	aiColor4D specular(0, 0, 0, 1);
	aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular);

	float shininess = 0.0;
	unsigned int max;
	aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);

	Material material(diffuse, specular, shininess, textured ? 1 : 0);
		

	glGenBuffers(1, &matUniformIndex);
	glBindBuffer(GL_UNIFORM_BUFFER, matUniformIndex);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &material,
		GL_STATIC_DRAW);
}

void Mesh::renderMesh(){

	glBindBufferRange(GL_UNIFORM_BUFFER, shaderMaterialUniformLocation,
		matUniformIndex, 0, sizeof(Material));	
	// bind texture
	glBindTexture(GL_TEXTURE_2D, texturesId[textureName]);
	// bind VAO
	glBindVertexArray(vao);
	// draw
	glDrawElements(GL_TRIANGLES,faces.size() * 3,GL_UNSIGNED_INT,0);
}

Mesh::~Mesh(){

	if (positions.size() > 0)
		glDeleteBuffers(1,&positions_vbo);

	if (normals.size() > 0)
		glDeleteBuffers(1,&normals_vbo);

	if (texcoords.size() > 0)
		glDeleteBuffers(1,&texcoords_vbo);

	glDeleteBuffers(1, &matUniformIndex);
	glDeleteVertexArrays(1, &vao);
}