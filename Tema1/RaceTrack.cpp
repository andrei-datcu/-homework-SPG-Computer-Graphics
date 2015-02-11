#define _USE_MATH_DEFINES

#include "RaceTrack.h"
#include <glew/glew.h>
#include <tuple>
#include <algorithm>
#include <random>
#include <cmath>
#include "Object3D.h"

unsigned int RaceTrack::loc_num_points, RaceTrack::loc_center;


static float truncf(float x){ //should be in cmath, but it isn't. Go Microsoft
	return x > 0 ? floorf(x) : ceilf(x);
}

static float det(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3){
	return p1.x*p2.y+p1.y*p3.x+p2.x*p3.y-p3.x*p2.y-p3.y*p1.x-p2.x*p1.y;
}

std::vector<glm::vec2> computeConvexHull(const std::vector<glm::vec2> &points){

	/**
	  * Metoda care calculeaza infasuratoarea convexa folosind scanrea 
	  * graham.
	**/

	std::vector <std::tuple<glm::vec2, float, float>> temp;

	int certainPos = 0;

	for (auto &p : points){
		temp.push_back(std::make_tuple(p, 0, 0));
		if (p.x < std::get<0>(temp[certainPos]).x ||
             (p.x == std::get<0>(temp[certainPos]).x) &&
             (p.y == std::get<0>(temp[certainPos]).y))
	  	    certainPos = temp.size() - 1;
	}

	auto aux = temp[certainPos]; 
	temp[certainPos] = temp[0];
	temp[0] = aux;

	glm::vec2 certain = std::get<0>(temp[0]), pct;

	for (auto &e : temp){
		pct = std::get<0>(e);
		if (abs(pct.x - certain.x) <FLT_EPSILON)
			std::get<2>(e) = FLT_MAX;
		else
			std::get<2>(e) = (pct.y - certain.y) / (pct.x - certain.x);
		
		std::get<1>(e) = glm::distance(pct, certain);
	}

	std::get<2>(temp[0]) = -FLT_MAX;

	std::sort(temp.begin(), temp.end(),
		[](std::tuple<glm::vec2, float, float> e1,
			std::tuple<glm::vec2, float, float> e2){

				float p1 = std::get<2>(e1), p2 = std::get<2>(e2);
				float d1 = std::get<1>(e1), d2 = std::get<1>(e2);

				return p1 < p2 || (p1 == p2 && d1 < d2);
	});

	std::vector<glm::vec2> convexHull;
	
	glm::vec2 p = std::get<0>(temp[0]);

	convexHull.emplace_back(p);
	convexHull.emplace_back(std::get<0>(temp[1]));

	for (int i = 2; i < temp.size();)
		if (truncf(det(convexHull[convexHull.size() - 2],
				convexHull[convexHull.size() - 1], std::get<0>(temp[i]))) < 0)

			convexHull.pop_back();
		else
			convexHull.push_back(std::get<0>(temp[i++]));

    return convexHull;
}


static std::random_device rd; // obtain a random number from hardware
static std::mt19937 generator(rd());

std::vector<glm::vec2> generate_random_points (
    unsigned int count, float width, float height)
{
    std::uniform_real_distribution<float> xd(-width / 2, width / 2);
	std::uniform_real_distribution<float> yd(-height / 2, height / 2);

    std::vector<glm::vec2> result;
    for (unsigned int i = 0; i < count; ++i)
        result.emplace_back(xd(generator), yd(generator));
    return result;
}

static void push_apart(std::vector<glm::vec2> &dataSet)  
{  
    const float dst = 222;
    for(int i = 0; i < dataSet.size() - 2; ++i)
        for (int j = i + 1; j < dataSet.size() - 1; ++j)
            if(glm::distance(dataSet[i], dataSet[j]) < dst) {  
                glm::vec2 forward = glm::normalize(dataSet[j] - dataSet[i]);
                glm::vec2 right = glm::normalize(glm::vec2(-forward.y, forward.x));
                dataSet[j] -= right * 20.0f * dst;
            }
}

static void add_curbes(std::vector<glm::vec2> &points)
{
    const float dst = 18222;
    for(int i = 0; i < points.size() - 1; ++i)      
            if(glm::distance(points[i], points[(i + 1) % points.size()]) > dst) {  
                glm::vec2 forward = glm::normalize(points[i + 1] - points[i]);
                glm::vec2 right = glm::normalize(glm::vec2(-forward.y, forward.x));
                points.insert(points.begin() + i + 1, points[i] + forward / 2.0f + right * 0.2f * dst);
                ++i;
            }
}

RaceTrack::RaceTrack():
    last_projection_s(0)
{
    std::vector<glm::vec2> ch = computeConvexHull(
        generate_random_points(10000, 25000, 25000));

    //push_apart(ch);
    add_curbes(ch);
    //push_apart(ch);


    control_points.emplace_back(ch.front().x, 0, ch.front().y);
    static std::uniform_real_distribution<float> hd(-M_PI / 6, M_PI / 6);
    const float max_height = 400;
    for (int i = 1; i < ch.size(); ++i){
        float height = control_points.back().y +
            glm::distance(ch[i], ch[i - 1]) * sin(hd(generator));

        if (abs(height) > max_height) height = (height > 0 ? 1.0f : -1.0f) *
            max_height;
        control_points.emplace_back(ch[i].x, height , ch[i].y);
    }

    track = new CatmullTrack(control_points);

    prepare_texture();
    // generez VAO
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);

    positions_vbo = generateVBO(track->positions, 0);
    tex_vbo = generateVBO(tex_coords, 1);
    normals_vbo = generateVBO(track->normals, 2);

    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void RaceTrack::render()
{
    glBindTexture(GL_TEXTURE_2D, texid);
    glBindVertexArray(vao);
    glUniform1ui(loc_num_points, num_points);
    glDrawArrays(GL_LINE_LOOP, 0, track->positions.size());
}

glm::vec3 RaceTrack::get_start_pos()
{
    return track->positions[0];
}

glm::vec3 RaceTrack::get_position(float s)
{
    return track->eval_by_arc_length(s);
}

float RaceTrack::project_on_axis(const glm::vec3 &point, float estimate)
{
    if (estimate == 0) estimate = last_projection_s;
    float s = track->projection(point, last_projection_s);
    if (s > length()) s -= length();
    if (s < 0) s+= length();

    last_projection_s = s;
    return s;
}

void RaceTrack::prepare_texture()
{
    texid = loadImage("resurse\\road2.jpg");

    const float strip_size = 350.0f;

    tex_coords.reserve(track->positions.size());
    tex_coords.clear();

    float tmp;

    for (int i = 0; i < track->positions.size(); ++i)
        tex_coords.push_back(modf(((float)i * length() / (float)track->render_samples) / strip_size, &tmp));
}

float RaceTrack::length()
{
    return track->arc_lengths.back();
}