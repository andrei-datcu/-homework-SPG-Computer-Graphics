#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glew/glew.h>

#include "CatmullTrack.h"

class RaceTrack
{
public:
    RaceTrack();
    void render();

    glm::vec3 get_start_pos();
    glm::vec3 get_position(float s);
    float length();
    float project_on_axis(const glm::vec3& pos, float estimate=0);

    static unsigned int loc_num_points, loc_center;
private:

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

    void prepare_texture();

    CatmullTrack *track;
    unsigned int vao, positions_vbo, normals_vbo, tex_vbo;
    std::vector<glm::vec3> control_points;

    std::vector<float> tex_coords;
    unsigned int texid;

    unsigned int num_points;
    glm::vec3 center;
    
    float last_projection_s;
};

