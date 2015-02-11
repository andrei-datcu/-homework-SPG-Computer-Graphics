#pragma once

#include <vector>
#include <glew/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <unordered_map>

struct Material{
	aiColor4D diffuse, specular;
	float shinness;
	int texCount;

	Material(const aiColor4D &diffuse, const aiColor4D &specular,
		float shinness, int texCount): diffuse(diffuse), specular(specular),
		shinness(shinness), texCount(texCount){};
};

class Mesh{
public:
	Mesh(const aiMesh* assimpMesh, const aiScene *assimpScene);
	~Mesh();

	template <typename T>
	static unsigned int generateVBO(const std::vector<T> &buffer,
		const unsigned int pipe){
	
		unsigned int vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(T), &buffer[0],
			GL_STATIC_DRAW);
		glEnableVertexAttribArray(pipe);
		glVertexAttribPointer(pipe, sizeof(T) / sizeof(float),
			GL_FLOAT, GL_FALSE, 0, 0);
		return vbo;
	};

	void renderMesh();

	unsigned int vao;
	static unsigned int shaderMaterialUniformLocation;
	static std::unordered_map<std::string, unsigned int> texturesId;
	
private:
	unsigned int positions_vbo, normals_vbo, texcoords_vbo, ibo;
	unsigned int matUniformIndex, textureIndex;
	std::string textureName;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::uvec3> faces; //just triangles;

};
