//-------------------------------------------------------------------------------------------------
// Descriere: Cubemap editabil
//
// Autor: Andrei Datcu
// Data: 13.12.2014
//-------------------------------------------------------------------------------------------------

#pragma once

#include <glew/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>


class DynamicCubemap
{
public:
    DynamicCubemap(const glm::vec3 &center, int width, int height);
    ~DynamicCubemap(void);

    void bind_framebuffer();
    void bind_face(unsigned df, glm::mat4 &view_matrix, glm::mat4 &proj_matrix);
    void bind_texture();

private:
    glm::vec3 cube_center;
    unsigned int cubemap_texture, framebuffer, depthbuffer;

    void check_fbb();
};

