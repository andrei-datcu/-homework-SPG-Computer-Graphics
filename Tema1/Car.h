#pragma once
#include "object3d.h"
#include "RaceTrack.h"
#include "lab_camera.hpp"
#include <chrono>
class Car :
    public Object3D
{
public:
    Car(const unsigned int &location_model_matrix, const std::string &fileName,
        RaceTrack &track);

    void accelerate(float da);
    void steer(float dangle);
    std::pair<glm::vec3, glm::vec3> headlight_pos();
    glm::vec3 headlight_dir();

    void update_position();
    void renderObject();

    lab::Camera tps_back_camera;
    unsigned int laps_completed;
private:

    void update_camera();

    glm::vec3 current_speed;
    glm::vec3 acceleration;
    glm::vec3 lightdir;
    RaceTrack &track;
    glm::vec3 position, forward, up;
    float arclen, angle;
    const float car_length;

    std::chrono::high_resolution_clock::time_point last_render_time;
};

