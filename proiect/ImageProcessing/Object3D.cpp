#include "stdafx.h"
#include "Object3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <glew/glew.h>
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

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		meshes.emplace_front(scene->mMeshes[i], scene);
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
