//-------------------------------------------------------------------------------------------------
// Descriere: Sistemul de particule
//
// Autor: Andrei Datcu
// Data: 13.12.2014
//-------------------------------------------------------------------------------------------------

#include "ParticleSystem.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include <glew/glew.h>
#include "lab_shader_loader.hpp"
#include "lab_texture_loader.hpp"

#include <random>


ParticleSystem::ParticleSystem(unsigned int particle_count)
    :building_top(0, 5, 0), building_radius(2)
{
    render_shader =
        lab::loadShader("shadere\\particle_render_vertex.glsl",
        "shadere\\particle_render_geometry.glsl",
        "shadere\\particle_render_fragment.glsl");

    compute_shader = lab::loadShader("shadere\\particle_compute.glsl");

    textura = lab::loadTextureBMP("resurse\\raindrop.bmp");

    //structura particule
    struct Particula{
        glm::vec4 position;
        glm::vec4 speed;
        glm::vec4 initialpos;
        glm::vec4 initialspd;
        Particula(const glm::vec4 &pos, float rety, const glm::vec4 &spd){
            position = pos;
            speed = spd;
            initialpos = pos;
            initialspd= spd;
            initialpos.y = rety;
        }
    };

    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_real_distribution<float> posd(-290, 290);
    std::uniform_real_distribution<float> speed(-50, -40);
    std::uniform_real_distribution<float> resy(290, 300);
   
    //genereaza particule start
    std::vector<Particula> vertecsi;
    for(unsigned int i=0;i<particle_count;i++){
        float pos_x = posd(generator);
        float pos_y = posd(generator);
        float pos_z = posd(generator);
        glm::vec4 pos = glm::vec4(pos_x, pos_y, pos_z, 1);

        if (sqrt( pow((pos_x - building_top.x), 2) + pow((pos_z - building_top.z), 2)) < building_radius)
            if (pos_y < building_top.y) continue;

        glm::vec4 spd = glm::vec4( 0,speed(generator),0 ,0);
        vertecsi.push_back(Particula(pos,resy(generator), spd));
    }

    //adauga indecsi
    std::vector<unsigned int> indices;
    for(unsigned int i=0;i<  vertecsi.size();i++) indices.push_back(i);

    glGenVertexArrays(1, &particle_vao);
    glBindVertexArray(particle_vao);

    //vertex buffer object -> un obiect in care tinem vertecsii
    glGenBuffers(1,&particle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, particle_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertecsi.size()*sizeof(Particula), &vertecsi[0], GL_STATIC_DRAW);

    //index buffer object -> un obiect in care tinem indecsii
    glGenBuffers(1,&particle_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particle_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    //legatura intre atributele vertecsilor si pipeline, datele noastre sunt INTERLEAVED.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(Particula),(void*)0);						//trimite pozitii pe pipe 0
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Particula),(void*)(sizeof(float)*4));		//trimite speed pe pipe 1

    num_indices = indices.size();

    glBindVertexArray(0);
}


ParticleSystem::~ParticleSystem(void)
{
    glDeleteBuffers(1,&particle_vbo);
    glDeleteBuffers(1,&particle_ibo);
    glDeleteVertexArrays(1,&particle_vao);
}

void ParticleSystem::bind()
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0, particle_vbo);
	glBindVertexArray(particle_vao);
    glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, textura);
}

void ParticleSystem::set_render_shader_vars(const glm::mat4 &proj_matrix,
        const glm::mat4 &view_matrix)
{
    glm::mat4 &model_matrix = glm::mat4(1);
    glUseProgram(render_shader);
    glUniformMatrix4fv(glGetUniformLocation(render_shader,"view_matrix"),1,false,glm::value_ptr(view_matrix));
	glUniformMatrix4fv(glGetUniformLocation(render_shader,"projection_matrix"),1,false,glm::value_ptr(proj_matrix));
	glUniformMatrix4fv(glGetUniformLocation(render_shader,"model_matrix"),1,false,glm::value_ptr(model_matrix));
    glUniform3f(glGetUniformLocation(render_shader, "building_pos"), building_top.x, building_top.y, building_top.z);
    glUniform1f(glGetUniformLocation(render_shader, "building_radius"), building_radius);
    glUniform1i( glGetUniformLocation(render_shader, "textura"),1 );
}

void ParticleSystem::draw()
{
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    glDrawElements(GL_POINTS, num_indices, GL_UNSIGNED_INT, 0);
    glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void ParticleSystem::update_positions()
{
    bind();
    glUseProgram(compute_shader);
    glDispatchCompute(num_indices / 512 + 1, 1, 1);
}
