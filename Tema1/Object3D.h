#pragma once
#include <string>
#include <forward_list>
#include "Mesh.h"
#include "lab_camera.hpp"
#include <glm/glm.hpp>
#include <assimp\scene.h>

class Object3D{
public:
	Object3D(const unsigned int &location_model_matrix,
		const glm::mat4 &original_model_matrix, const std::string &fileName);
	void loadFromFile(const std::string &fileName);
	static void loadTextures(const aiScene *assimpScene);
	void renderObject();
	void translate(glm::vec3 v);
	void rotate(float angle, glm::vec3 rotdirection);
	void setTranslation(glm::vec3 v);
	void setRotation(float angle, glm::vec3 rotdirection);


protected:
	float rotationAngle;
	glm::mat4 original_model_matrix, model_matrix;
	glm::vec3 translationVector;
		
private:
	std::forward_list<Mesh> meshes;
	unsigned int location_model_matrix; //locatia in vertex shader
};

unsigned int loadImage(std::string filename);
