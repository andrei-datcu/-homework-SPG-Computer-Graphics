#include "Object3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <glew/glew.h>
#include <IL/il.h>

#include <iostream>

#define directory "resurse\\"
#define PI 3.14159265359f


Object3D::Object3D(const unsigned int &location_model_matrix,
		const glm::mat4 &original_model_matrix, const std::string &fileName):
		
			location_model_matrix(location_model_matrix),
			original_model_matrix(original_model_matrix),
			model_matrix(original_model_matrix),
			translationVector(0, 0, 0),
			rotationAngle(0){

	loadFromFile(fileName);
}

void Object3D::loadFromFile(const std::string &fileName){

	meshes.clear();
	
	Assimp::Importer importer;

	//importez obiectul folosind assimp
	const aiScene *scene = importer.ReadFile( fileName,
		aiProcessPreset_TargetRealtime_Fast);

    std::cerr << importer.GetErrorString();

	//incarc toate texturile disponibile
	Object3D::loadTextures(scene);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		meshes.emplace_front(scene->mMeshes[i], scene);
}

unsigned int loadImage(std::string filename) {

	ILboolean success;
	unsigned int imageID;
 
	// initializz DevIL.
	ilInit();
	// generez un id pentru imagine
	ilGenImages(1, &imageID); 
	// Bind
	ilBindImage(imageID); 
	// Pun originea la fel cum e in OpenGL
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	//incarc imaginea
	success = ilLoadImage((ILstring)filename.c_str());
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	//totul a fost ok?
	if (!success) {
		ilDeleteImages(1, &imageID); 
		return 0;
	}

	ilBindImage(imageID);
	int w = ilGetInteger(IL_IMAGE_WIDTH);
	int h = ilGetInteger(IL_IMAGE_HEIGHT);
	unsigned char* data = ilGetData();
	unsigned int textureID;
	glGenTextures(1, &textureID); // Generez id pentru textura
	glBindTexture(GL_TEXTURE_2D, textureID); 

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ); 
	float maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  w, h, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, data); 
	glGenerateMipmap(GL_TEXTURE_2D);
	
	//Putem sterge imaginea il, ea aflandu-se acum in bufferul OpenGl
	ilDeleteImages(1, &imageID); 
	return textureID;
}

void Object3D::loadTextures(const aiScene *scene){

/*
 * Metoda care incarca toate texturile dintr-o scena assimp
 */

	//toate materialele din scena
	for (unsigned int m = 0 ; m < scene->mNumMaterials; ++m){

		int texIndex = 0;
		aiString path;	// filename

		while (scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE,
			texIndex, &path) == AI_SUCCESS) {
			Mesh::texturesId[path.data] =
				loadImage(std::string(directory +
					std::string(path.data)).c_str()); 
			texIndex++;
		}
	}
}

void Object3D::translate(glm::vec3 v){

	translationVector += v;
	model_matrix =  glm::rotate(glm::translate(original_model_matrix,
		translationVector), rotationAngle * 180 / PI, glm::vec3(0, 1, 0));
}

void Object3D::rotate(float dangle, glm::vec3 rotdirection){
	
	rotationAngle += dangle;
	model_matrix =  glm::rotate(glm::translate(original_model_matrix,
		translationVector), rotationAngle * 180 / PI, rotdirection);
}


void Object3D::renderObject(){

	glUniformMatrix4fv(location_model_matrix, 1, false,
		glm::value_ptr(model_matrix));

	for (Mesh &m : meshes)
		 m.renderMesh();
}

void Object3D::setTranslation(glm::vec3 v){

	translationVector = v;
	translate(glm::vec3(0, 0, 0));
}

void Object3D::setRotation(float angle, glm::vec3 rotdirection){

	rotationAngle = angle;
	rotate(0, rotdirection);
}
