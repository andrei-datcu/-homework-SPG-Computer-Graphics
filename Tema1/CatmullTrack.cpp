#include <vector>
#include <glm/glm.hpp>
#include "CatmullTrack.h"
#include <iostream>
#include <map>

CatmullTrack::CatmullTrack(const std::vector<glm::vec3> &control_points, 
        int samples, int render_samples) : samples(samples), render_samples(render_samples)
{
    int n = control_points.size();
    positions.reserve(render_samples);
    normals.reserve(render_samples);
    arc_lengths.reserve(n * samples);
    splines.reserve(n * samples);

    float fsamples = (float)samples;
    float d = 0;
    glm::vec3 last_point;

    for (int i = 0; i < n; ++i) {
        CatmullSpline spline(control_points[(i + n - 1) % n], control_points[i],
            control_points[(i + 1) % n], control_points[(i + 2) % n]);

        for (int j = 0; j < samples; ++j) {
            glm::vec3 new_point = spline.eval((float)j/fsamples);
            if (i > 0 || j > 0)
                d += glm::distance(new_point, last_point);
            last_point = new_point;
            arc_lengths.push_back(d);
        }
        splines.push_back(spline);
    }

    arc_lengths.push_back(arc_lengths.back() + glm::distance(splines.front().eval(0), last_point));
    d = arc_lengths.back();
    std::cerr<<arc_lengths.back() << std::endl;

    float frender_sample = (float)render_samples;

    for (int i = 0; i < render_samples; ++i){
        auto pair = t_from_arc_length((float)i * d/frender_sample); 
        float t = pair.second;
        positions.push_back(splines[pair.first].eval(t));
        normals.push_back(glm::normalize(splines[pair.first].eval_d(t)));
    }
}


std::pair<int, float> CatmullTrack::t_from_arc_length(float s)
{
    //Mai intai il cautam binar pe s in arc_lengths

    if (s >= arc_lengths.back())
        s -= arc_lengths.back();

    if (s < 0)
        s += arc_lengths.back();

    int i, step, n = arc_lengths.size() - 1;
    for (step = 1; step < n; step <<= 1);
    for (i = 0; step; step >>= 1)
        if (i + step < n && arc_lengths[i + step] <= s)
           i += step;
    
    assert(i < arc_lengths.size() - 1);

    float lengthBefore = arc_lengths[i];
    float lengthAfter = arc_lengths[i+1];
    float segmentLength = lengthAfter - lengthBefore;

    // determine where we are between the 'before' and 'after' points.
    float segmentFraction = (s - lengthBefore) / segmentLength;
                          
    // add that fractional amount to t
    int spline_index = i / samples;
    float t = (i % samples + segmentFraction) / (float) (samples);
    return std::make_pair(spline_index, t);
}

glm::vec3 CatmullTrack::eval_by_arc_length(float s)
{
    auto t = t_from_arc_length(s);
    return splines[t.first].eval(t.second);
}

//functie care intoarce proiectia punctului point sub
//forma de argument arc_length
float CatmullTrack::projection(const glm::vec3 &point, float initial_estimate)
{
    //aplicam formula lui newton si incercam si gasim s a.i distanta dintre
    //punctul point si cel de pe spline sa fie minima
    register float prev, current = initial_estimate;
    register int iter = 0;
    do {
        prev = current;
        auto t = t_from_arc_length(prev);
        glm::vec3 d = splines[t.first].eval(t.second);
        glm::vec3 dd = splines[t.first].eval_d(t.second);
        glm::vec3 dd2 = splines[t.first].eval_d2(t.second);

        float D = 2.0f * (d.x - point.x) * dd.x + 2.0f * (d.y - point.y) * dd.y +
            2.0f * (d.z - point.z) * dd.z;

        float D2 = 2.0f * dd.x * dd.x + 2.0f * (d.x - point.x) * dd2.x +
            2.0f * dd.y * dd.y + 2.0f * (d.y - point.y) * dd2.y +
            2.0f * dd.z * dd.z + 2.0f * (d.z - point.z) * dd2.z;
        double cd = ((double)prev * D2 - (double)D) / (double)D2;
        //current = prev - D/D2;
        current = (float)cd;
        ++iter;
    } while (abs(current - prev) > 1e-3 && iter <= 50000);

    return current;
}