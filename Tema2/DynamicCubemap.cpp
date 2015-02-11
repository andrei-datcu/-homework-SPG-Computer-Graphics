//-------------------------------------------------------------------------------------------------
// Descriere: Cubemap editabil
//
// Autor: Andrei Datcu
// Data: 13.12.2014
//-------------------------------------------------------------------------------------------------

#include "DynamicCubemap.h"

DynamicCubemap::DynamicCubemap(const glm::vec3 &center, int width, int height)
    : cube_center(center)
{
    glGenTextures(1, &cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);


    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer);


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemap_texture, 0);

    // disable
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


DynamicCubemap::~DynamicCubemap(void)
{
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &depthbuffer);
}

void DynamicCubemap::bind_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void DynamicCubemap::bind_face(unsigned df, glm::mat4 &view_matrix, glm::mat4 &proj_matrix)
{
    int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)df;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, cubemap_texture, 0);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   proj_matrix = glm::perspective(90.0f, 1.0f, 1.0f, 301.0f);

    switch (face)
    {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        view_matrix = glm::lookAt(cube_center, glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
        break;

    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        view_matrix = glm::lookAt(cube_center, glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
        break;

    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        view_matrix = glm::lookAt(cube_center, glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        break;

    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        view_matrix = glm::lookAt(cube_center, glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
        break;

    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        view_matrix = glm::lookAt(cube_center, glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
        break;

    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        view_matrix = glm::lookAt(cube_center, glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
        break;
    };
}

void DynamicCubemap::bind_texture()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
}

void DynamicCubemap::check_fbb()
{
    GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (e != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "There is a problem with the FBO: ";

#define CASE_DBG_MACRO(x) \
    case x: std::cerr << "" #x << std::endl; break

        switch (e) {
        CASE_DBG_MACRO(GL_FRAMEBUFFER_UNDEFINED);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_UNSUPPORTED);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        CASE_DBG_MACRO(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
        default: std::cerr<<"wtf!!!" << std::endl; break;
        }
    }
#undef CASE_DBG_MACRO

}
