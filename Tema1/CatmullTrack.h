#pragma once

#include "CatmullSpline.h"
#include <vector>

class CatmullTrack
{
public:
    CatmullTrack(const std::vector<glm::vec3> &control_points, 
        int samples=16300, int render_samples = 6000);//1200

    const int render_samples;

    std::vector<glm::vec3> positions; //vector de pozitii intermediare folosit la randat
    std::vector<glm::vec3> normals;
    std::vector<float> arc_lengths;

    glm::vec3 eval_by_arc_length(float s);
    
    float projection(const glm::vec3 &point, float initial_estimate);

private:

    std::pair <int, float> t_from_arc_length(float s);

    const int samples; //sample points per spline
    std::vector<CatmullSpline> splines;
};

