#include "stdafx.h"
#include "Canny.h"
#include <array>
#include <iostream>
#include <queue>

#include "Gaussian.h"

static void draw_line(int x1, int y1, int x2, int y2, Image &img);

static int dir_from_rad(double rad)
{
#define BETWEEN(x, A, B) ((x) >= (A) && (x) <= (B))

    if (rad >= 7 * M_PI / 8 ||  rad <=  - 7 * M_PI / 8)
        return 2;

    if (BETWEEN(rad, -M_PI/8, M_PI/8))
        return 2;

    if (BETWEEN(rad, -5 * M_PI / 8, - 3 * M_PI / 8) ||
        BETWEEN(rad, 3 * M_PI / 8, 5 * M_PI / 8))
        return 0;
    
    if (BETWEEN(rad, -7 * M_PI / 8,-5 * M_PI / 8) ||
        BETWEEN(rad, M_PI / 8, 3 * M_PI / 8))
        return 1;

    if (BETWEEN(rad, 5 * M_PI / 8, 7 * M_PI / 8) ||
        BETWEEN(rad, -3 * M_PI / 8, - M_PI / 8))
        return 3;

#undef BETWEEN
}

static std::vector<bool> canny_bin(const Image &img, unsigned tl, unsigned th)
{

    std::vector<double> gauss = generate_gaussian_kernel(0.9);//0.9
    double denom = calc_kernel_denom(gauss);

    //Calculam gradientii pe cele doua directii

    Image gx(img);

    gx.convolve(gauss.data(), 1, gauss.size(), denom);
    gx.convolve(gauss.data(), gauss.size(), 1, denom);

    Image gy(gx);

    double sobelx[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    double sobely[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};

    gx.convolve(sobelx, 3, 3);
    gy.convolve(sobely, 3, 3);


    //Imaginea finala, care va contine punctele edge tari - binara
    std::vector<bool> strong_edge(gx.width * gx.height, false);

    //2 randuri de imagine cu puncte edge tari
    std::array<std::vector<boolean>, 2> weak_edge;
    weak_edge[0].resize(gx.width);
    weak_edge[1].resize(gx.width);

    //3 randuri din imagine de magnitudine
    std::array<std::vector<double>, 3> mag;
    mag[0].resize(gx.width);
    mag[1].resize(gx.width);
    mag[2].resize(gx.width);

    const int dx[4] = {-1, -1, 0, 1};
    const int dy[4] = {0, -1, -1, -1};

    int border = gauss.size() / 2 + 1;

    //Magnitudinea pentru primul rand
    for (int i = 0, p = border * gx.width; i < gx.width; ++i, ++p) {
        mag[0][i] = sqrt(gx[p] * gx[p] + gy[p] * gy[p]);
        int si = p + gx.width;
        mag[1][i] = sqrt(gx[si] * gx[si] + gy[si] * gy[si]);
    }

    for (int i = border + 1; i < gx.height - border; ++i) {

        //Calculam M si pentru un rand in fata
        for (int j = 0, p = (i + 1) * gx.width; j < gx.width; ++j, ++p)
            mag[2][j] = sqrt(gx[p] * gx[p] + gy[p] * gy[p]);

        if (i == 77 || i == 78)
            std::cerr << mag[1][299] << " " << mag[1][300] << std::endl;

        for (int j = border; j < gx.width - border; ++j) {

            int pointer = i * gx.width + j;
            //strong_edge[pointer] = mag[1][j]; continue;
            //Calculam directia muchiei = dir_gradient + pi/2
            /*double alpha = gx[pointer] == 0 ? 0 : atan(gy[pointer] / gx[pointer]) + M_PI_2;

            int cad = 2 * alpha / M_PI_4; //impartim 180 de grade in 8 cadrane
            int dir = ((cad + 1) % 8) / 2;
            */
            int dir = dir_from_rad(atan2(gy[pointer], gx[pointer]));
            //Non-maxima supression
            if (mag[1][j] >= mag[1 + dy[dir]][j + dx[dir]] &&
               mag[1][j] >= mag[1 - dy[dir]][j - dx[dir]]) {

                // Punctul este acceptat -- acum face dublu threhsolding

                if (mag[1][j] >= th) {
                    strong_edge[pointer] = 255;
                    // Cautam puncte slabe vecine ca sa le facem tari
                    for (int dd = 0; dd < 2; ++dd)
                        if (weak_edge[1 + dy[dd]][j + dx[dd]])
                            strong_edge[pointer + dy[dd] * gx.width + dx[dd]] = 255;

                } else if (mag[1][j] >= tl) {

                    weak_edge[1][j] = true;
                    int candidates[3] = {pointer - 1, pointer - gx.width};
                    candidates[2] = candidates[1] - 1;

                    for (int dd = 0; dd < 2; ++dd)
                        if (strong_edge[candidates[dd]]) {
                            strong_edge[pointer] = 255;
                            break;
                        }
                } //else > tl

            }

        }

        mag[0].swap(mag[1]);
        mag[1].swap(mag[2]);
        weak_edge[0].swap(weak_edge[1]);
    }

    return strong_edge;
}

static int fill_edge(const std::vector<bool> &edges,
                          std::vector<unsigned int> &visited,
                          unsigned int vvalue,
                          int width,
                          int starting_point)
{
    std::queue<int> q;

    static const int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    static const int dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};

    //Nu facem verificarea iesirii din imagine,
    //pentru ca presupunem ca imaginea nu are contur pe margine

    visited[starting_point] = vvalue;
    q.push(starting_point);
    int p;
    while (!q.empty()) {
        p = q.front();
        q.pop();

        for (int dir = 0; dir < 8; ++dir) {
            int pv = p + dy[dir] * width + dx[dir];
            if (edges[pv] && visited[pv] == 0) {
                q.push(pv);
                visited[pv] = vvalue;
            }
        }

    }

    return p;
}

static void draw_line(int x1, int y1, int x2, int y2,
                      std::vector<bool> &bin_img, int imgw)
{
    // Bresenham's line algorithm
    const bool steep = (abs(y2 - y1) > abs(x2 - x1));
    if(steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if(x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    const int dx = x2 - x1;
    const int dy = abs(y2 - y1);

    int error = dx;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;

    const int maxX = (int)x2;

    for(int x = x1; x < maxX; x++) { 
        if(steep)
            bin_img[x * imgw + y] = 255;
        else
            bin_img[y * imgw + x] = 255;

        error -= 2 * dy;
        if(error < 0) {
            y += ystep;
            error += 2 * dx;
        }
    }

}
 
static void link_edges(std::vector<bool> &bin_img, int imgw)
{

    std::vector<unsigned int> visited(bin_img.size(), 0);

    const int max_range = 5;
    unsigned int cc = 0; //nr. componenta conexa

    for (int i = 0; i < bin_img.size(); ++i)
        if (bin_img[i] && visited[i] == 0) {

            //Avem un nou edge-point prin care n-am mai trecut
            int start = i;

            do {
                int p = fill_edge(bin_img, visited, ++cc, imgw, start);

                int dist_cand = 2 * max_range * max_range + 1;
                start = -1;
                for (int r = -max_range; r <= max_range; ++r)
                    for (int rc = -max_range, pp = p + r * imgw - max_range;
                        rc <= max_range; ++rc, ++pp)
                        if (bin_img[pp] && visited[pp] != cc) {
                            int dist = r * r + rc * rc;
                            if (dist < dist_cand) {
                                dist_cand = dist;
                                start = pp;
                            }
                        }
                    if (dist_cand <= 1) break;
                    if (start != -1)
                        draw_line(p % imgw, p / imgw, start % imgw,
                                      start / imgw, bin_img, imgw);

            } while (start != -1);
        }
}

Image fill_main_obj(const Image &img)
{
    std::vector<bool> bin_img = canny_bin(img, 10, 30);
    link_edges(bin_img, img.width);

    Image result(img.width, img.height);

    for (unsigned i = 0; i < img.height * img.width; ++i)
        result[i] = bin_img[i] * 255;

    result.smart_fill(0, 0, 100);

    for (unsigned i = 0; i < result.width * result.height; ++i)
        if (result[i] == 0) {
            result.smart_fill(i % result.width, i / result.width, 255);
            break;
        }

    result.smart_fill(0, 0, 0);

    return result;
}