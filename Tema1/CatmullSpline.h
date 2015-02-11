#pragma once

#include <glm/glm.hpp>

struct CubicPoly
{
    float c0, c1, c2, c3;

    CubicPoly(){};
    CubicPoly(float x0, float x1, float t0, float t1);
    
    float eval(float t);
    float eval_d(float t);
    float eval_d2(float t);
};

class CatmullSpline
{
public:
    CatmullSpline(const glm::vec3& p0, const glm::vec3& p1, 
                  const glm::vec3& p2, const glm::vec3& p3);

    glm::vec3 eval(float t);
    glm::vec3 eval_d(float t);
    glm::vec3 eval_d2(float t);
private:
    CubicPoly px, py, pz;

    static CubicPoly init_nonuniform_cr(float x0, float x1, float x2, float x3,
        float dt0, float dt1, float dt2);
};

