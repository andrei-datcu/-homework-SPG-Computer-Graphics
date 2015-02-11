#include "CatmullSpline.h"
#include <cassert>

CatmullSpline::CatmullSpline(const glm::vec3& p0, const glm::vec3& p1, 
                             const glm::vec3& p2, const glm::vec3& p3)
{
    float dt0 = sqrtf(glm::distance(p0, p1));
    float dt1 = sqrtf(glm::distance(p1, p2));
    float dt2 = sqrtf(glm::distance(p2, p3));

    // safety check for repeated points
    if (dt1 < 1e-4f)    dt1 = 1.0f;
    if (dt0 < 1e-4f)    dt0 = dt1;
    if (dt2 < 1e-4f)    dt2 = dt1;

    px = init_nonuniform_cr(p0.x, p1.x, p2.x, p3.x, dt0, dt1, dt2);
    py = init_nonuniform_cr(p0.y, p1.y, p2.y, p3.y, dt0, dt1, dt2);
    pz = init_nonuniform_cr(p0.z, p1.z, p2.z, p3.z, dt0, dt1, dt2);
}


glm::vec3 CatmullSpline::eval(float t)
{
    return glm::vec3(px.eval(t), py.eval(t), pz.eval(t));
}

glm::vec3 CatmullSpline::eval_d(float t)
{
    return glm::vec3(px.eval_d(t), py.eval_d(t), pz.eval_d(t));
}

glm::vec3 CatmullSpline::eval_d2(float t)
{
    return glm::vec3(px.eval_d2(t), py.eval_d2(t), pz.eval_d2(t));
}

CubicPoly CatmullSpline::init_nonuniform_cr(
    float x0, float x1, float x2, float x3, float dt0, float dt1, float dt2)
{
    // calculeaza tagentele in [t1,t2]
    float t1 = (x1 - x0) / dt0 - (x2 - x0) / (dt0 + dt1) + (x2 - x1) / dt1;
    float t2 = (x2 - x1) / dt1 - (x3 - x1) / (dt1 + dt2) + (x3 - x2) / dt2;

    // scaleaza tangentele pentru [0,1]
    t1 *= dt1;
    t2 *= dt1;

    return CubicPoly(x1, x2, t1, t2);
}

/*
 * Calculeaza coeficientii pentru polinomul de ord p 
 *   p(s) = c0 + c1*s + c2*s^2 + c3*s^3
 * a.i
 *   p(0) = x0, p(1) = x1
 *  si
 *   p'(0) = t0, p'(1) = t1
*/
CubicPoly::CubicPoly(float x0, float x1, float t0, float t1)
{
    c0 = x0;
    c1 = t0;
    c2 = -3*x0 + 3*x1 - 2*t0 - t1;
    c3 = 2*x0 - 2*x1 + t0 + t1;
}

float CubicPoly::eval(float t)
{
    assert(t >= 0.0f && t <= 1.0f);

    float t2 = t*t;
    float t3 = t2 * t;
    return c0 + c1*t + c2*t2 + c3*t3;
}

float CubicPoly::eval_d(float t)
{
    assert(t >= 0.0f && t <= 1.0f);
    return c1 + 2 * c2 * t + 3 * c3 * t * t;
}

float CubicPoly::eval_d2(float t)
{
    assert(t >= 0.0f && t <= 1.0f);
    return 2 * c2 + 6 * c3 * t;
}