#define _USE_MATH_DEFINES

#include "Car.h"
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>
#include <iostream>
#include <istream>

Car::Car(const unsigned int &location_model_matrix, const std::string &fileName,
         RaceTrack &track):
    Object3D(location_model_matrix, 
             glm::rotate(glm::mat4(), (float)M_PI, glm::vec3(0, 1, 0)),
             fileName),
    track(track),
    car_length(40.0f),
    laps_completed(0)
{
    arclen = 0;
    position = track.get_position(0);
    forward = glm::normalize(track.get_position(arclen + car_length) - position);

    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    up = glm::normalize(glm::cross(right, forward));

    update_camera();

    last_render_time = std::chrono::high_resolution_clock::now();
}

void Car::accelerate(float da)
{
    current_speed += da;
}

void Car::update_camera()
{
    tps_back_camera.set(position - forward * 120.0f + up * 65.0f, position, up);
}

void Car::steer(float dangle)
{
    forward = glm::normalize(glm::rotate(forward, dangle, up));
}

static std::ostream& operator << ( std::ostream &out, const glm::vec3 &v)
{
    out << "(" << v.x << ";" << v.y << ";" << v.z << ")";
    return out;
}

void Car::renderObject()
{
    model_matrix = glm::translate(glm::inverse(glm::lookAt(position, position + forward * car_length, up)), 25.5f * up.x, 25.5f * up.y, 25.5f * up.z);

    //model_matrix = glm::translate(glm::mat4(), position.x, position.y, position.z);
    
    Object3D::renderObject();

}

void Car::update_position()
{
    auto now = std::chrono::high_resolution_clock::now();
    float secs =
        std::chrono::duration_cast<std::chrono::duration<double>>
        (now - last_render_time).count();
    last_render_time = now;
    const float friction_acc = 0.5f;
    const float ds = 1e-2;
    const float track_width = 150.0f;

    glm::vec3 tmp = position + current_speed * forward;
    float proj_s = track.project_on_axis(tmp);//, arclen + glm::distance(tmp, position));
    glm::vec3 p = track.get_position(proj_s);

    if (glm::distance(tmp, p) > track_width) {
        current_speed = glm::vec3(0, 0, 0);
        return;
    }
    update_camera();
    glm::vec3 forward_b = glm::normalize(track.get_position(proj_s + car_length) - p);
    glm::vec3 right_b = glm::normalize(glm::cross(forward_b, glm::vec3(0, 1, 0)));
    up = glm::normalize(glm::cross(right_b, forward_b));
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    forward = glm::normalize(glm::cross(up, right));

    position = tmp;
    position.y = p.y;
    lightdir = glm::rotate(forward, -(float)M_PI, right);

    if (abs(arclen-proj_s - track.length()) < 100.0f)
        ++laps_completed;

    arclen = proj_s;
    

    current_speed.x = std::max(0.0f, current_speed.x - friction_acc *  secs);
    current_speed.y = std::max(0.0f, current_speed.y - friction_acc *  secs);
    current_speed.z = std::max(0.0f, current_speed.z - friction_acc *  secs);
}

std::pair<glm::vec3, glm::vec3> Car::headlight_pos()
{
    glm::vec3 right = glm::cross(up, forward);
    return std::make_pair(position - 30.0f * right + 5.5f * up, position + 30.0f * right + 5.5f * up);
}

glm::vec3 Car::headlight_dir()
{
    return lightdir;
}