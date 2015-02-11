//-------------------------------------------------------------------------------------------------
// Descriere: Sistemul de particule
//
// Autor: Andrei Datcu
// Data: 13.12.2014
//-------------------------------------------------------------------------------------------------

#pragma once

#include <glm/glm.hpp>

class ParticleSystem
{
public:
    ParticleSystem(unsigned int particle_count);
    ~ParticleSystem(void);

    void bind();
    void set_render_shader_vars(const glm::mat4 &proj_matrix,
        const glm::mat4 &view_matrix);
    void draw();
    void update_positions();

private:
    unsigned int particle_vao, particle_vbo, particle_ibo, num_indices;

    unsigned int compute_shader, render_shader, textura;
    const glm::vec3 building_top;
    const float building_radius;
};

