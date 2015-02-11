#include "stdafx.h"
#include "Image.h"
#include <math.h>
#include <stdlib.h>
#include <stack>
#include <tuple>
#include <queue>
#include <iostream>


Image::Image(unsigned char *from_corona, unsigned int width, unsigned int height)
    : pixels(width * height), width(width), height(height)
{
    for (unsigned i = 0, p = 0; i < width * height; ++i, p += 3)
        pixels[i] = from_corona[p];
}

Image::Image(unsigned int width, unsigned int height)
    : pixels(width * height, 0), width(width), height(height)
{
}

void Image::to_corona(unsigned char *data)
{
    // Functia presupune ca memoria a fost alocata deja
    
    for (unsigned i = 0, p = 0; i < width * height; ++i, p += 3) {
        data[p] = data[p + 1] = data[p + 2] = pixels[i];
    }

}

void Image::to_corona_red(unsigned char *data, std::vector<bool> &rc)
{
    for (unsigned i = 0, p = 0; i < width * height; ++i, p += 3) {
        data[p] = rc[i] ? 255 : pixels[i];
        data[p + 1] = data[p + 2] = pixels[i];
    }
}


void Image::convolve (double *mask, unsigned mask_width,
               unsigned mask_height, double denom)
{
	std::vector<int> fullres(width * height);

	unsigned xcenter = (mask_width - (1 - mask_width % 2)) / 2;
    unsigned ycenter = (mask_height - (1 - mask_height % 2)) / 2;

	unsigned i, j, di, dj;
	min = 10000;
    max=-10000;
	unsigned long pointer;

	for (i = ycenter; i < height - ycenter; ++i)
		for (j = xcenter; j < width - xcenter; ++j) {
			pointer = ((i - ycenter) * width + (j - xcenter));
			double register conv = 0;
			for (di = 0; di < mask_height; ++di, pointer += (width - mask_width))
				for (dj = 0; dj < mask_width; ++dj, ++pointer)
					conv += mask[di * mask_width + dj] * pixels[pointer];

            conv /= denom;
            int iconv = (int) conv;
			fullres[i * width+ j] = iconv;
		
            if (iconv < min)
				min = iconv;
			if (iconv > max)
				max = iconv;
		}

    pixels.swap(fullres);
}

void Image::addAbs(const Image& secondImage)
{
    min = 10000;
    max=-10000;

    for (unsigned i = 0; i < width * height; ++i) {
        pixels[i] = std::abs(pixels[i]) + std::abs(secondImage.pixels[i]);
        if (pixels[i] > max)
            max = pixels[i];
        if (pixels[i] < min)
            min = pixels[i];
    }
}

 void Image::abs()
 {

    min = 10000;
    max=-10000;

    for (int &p : pixels) {
        p = std::abs(p);

        if (p < min)
            min = p;
        if (p > max)
            max = p;
    }
 }

 void Image::normalize(unsigned xborder, unsigned yborder) 
 {
    double fact = 255.0/(max-min);
	for (unsigned i = yborder; i < height - yborder; ++i)
		for (unsigned j = xborder; j < width - xborder; ++j){
			double p = (double)(pixels[i * width + j] - min ) * fact;
			pixels[i * width + j] = (int)p;
		}
    
 }

 void Image::clip()
 {
     for (int &p : pixels)
         if (p > 255)
             p = 255;

 }

 int& Image::operator [](unsigned index)
 {
     return pixels[index];
 }

void Image::smart_fill(int x, int y, int new_color)
{

#define PUSH(XL, XR, Y, DY)	/* push new segment on stack */ \
    if (Y+(DY)>=0 && Y+(DY)< height) \
    {s.push(std::make_tuple(Y, XL, XR, DY));}

#define POP(XL, XR, Y, DY)	/* pop segment off stack */ \
    {std::tie(Y, XL, XR, DY) = s.top(); s.pop();Y += DY;}

#define GetPixel(x, y) pixels[y * width + x]
#define SetPixel(x, y, new_color) pixels[y * width + x] = new_color

    int left, x1, x2, dy;
    int old_color;
    std::stack<std::tuple<int, int, int, int>> s;

    old_color = GetPixel(x, y);

    PUSH(x, x, y, 1);        /* needed in some cases */
    PUSH(x, x, y+1, -1);    /* seed segment (popped 1st) */

    while( !s.empty() ) {
        POP(x1, x2, y, dy);

        for( x = x1; x >= 0 && GetPixel(x, y) == old_color; --x )
            SetPixel(x, y, new_color);

        if( x >= x1 )
            goto SKIP;

        left = x+1;
        if( left < x1 )
            PUSH(left, x1-1, y, -dy);    /* leak on left? */

        x = x1+1;

        do {
            for( ; x< width && GetPixel(x, y) == old_color; ++x )
                SetPixel(x, y, new_color);

            PUSH(left, x-1, y, dy);

            if( x > x2+1 )
                PUSH(x2+1, x-1, y, -dy);    /* leak on right? */

SKIP:        for( ++x; x <= x2 && GetPixel(x, y) != old_color; ++x );

            left = x;
        } while( x<=x2 );
    }
}

void Image::thin()
{
    std::vector<int> tmp(width * height, 0);

    bool changed;
    static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    int iter = 0;
    do {
        changed = false;
        ++iter;
        for (int c = 0; c < 4; ++c) {
            for (int i = 1; i < height - 1; ++i)
                for (int j = 1; j < width - 1; ++j) {
                    int pointer = i * width + j;
                    if (pixels[pointer] == 255) {

                        int np = 0; //nr pixeli de 1
                        int v[8];

                        for (int dir = 0; dir < 8; ++dir) {
                            int p = pointer + width * dy[dir] + dx[dir];
                            v[dir] = pixels[p];
                            np += (int)(v[dir] == 255);
                        }

                        int chi = (int)(v[0] != v[2]) + (int)(v[2]!=v[4]) + (int)(v[4] != v[6]) +
                            (int)(v[6] != v[0]) +
                            2 * ((int)((v[1] > v[0]) && (v[1] > v[2])) + (int)((v[3]>v[2]) && (v[3] > v[4]))
                            +(int)((v[5] > v[4]) && (v[5] > v[6])) + (int)((v[7] > v[6]) && (v[7] > v[0])));

                        if (2 <= np && np < 7 && chi == 2 && v[c * 2] == 0 && v[(c * 2 + 4) % 8] == 255) {
                                tmp[pointer] = 0;
                                changed = true;
                           // } else tmp[pointer] = 255;
                        } else tmp[pointer] = 255;
                    } // end if pixel == 255
                    else tmp[pointer] = 0;
                }
                pixels.swap(tmp);
        }
    } while (changed);
}

void Image::thin_std()
{
    std::vector<int> tmp(width * height, 0);

    bool changed;
    static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    const int cond[2][4] = {{2, 4, 6, 0}, {0, 6, 2, 4}};
    //const int cond[2][4] = {{0, 6, 2, 4}, {2, 4, 0, 6}};
    int iter = 0;
    do {
        changed = false;
        ++iter;
        for (int c = 0; c < 2; ++c) {
            for (int i = 1; i < height - 1; ++i)
                for (int j = 1; j < width - 1; ++j) {
                    int pointer = i * width + j;
                    if (pixels[pointer] == 255) {

                        int np = 0; //nr pixeli de 1
                        int nt = 0; //nr translatii 0->1
                        bool lastzero = pixels[pointer + width * dy[7] + dx[7]] == 0;

                        for (int dir = 0; dir < 8; ++dir) {
                            int p = pointer + width * dy[dir] + dx[dir];
                            if (pixels[p] == 255) {
                                if (lastzero) {
                                    ++nt;
                                    lastzero = false;
                                }
                                ++np;
                            } else lastzero = true;
                        }

                        if (3 <= np && np < 7 && nt == 1) {

                            int pointers[4];
                            for (int k = 0; k < 4; ++k)
                                pointers[k] = pointer + width * dy[cond[c][k]] + dx[cond[c][k]];

                            if (pixels[pointers[0]] == 0 || pixels[pointers[1]] == 0 ||
                                    (pixels[pointers[2]] == 0 && pixels[pointers[3]] == 0)) {
                                tmp[pointer] = 0;
                                changed = true;
                            } else tmp[pointer] = 255;
                        } else tmp[pointer] = 255;
                    } // end if pixel == 255
                    else tmp[pointer] = 0;
                }
                pixels.swap(tmp);
        }
    } while (changed);
}


Image Image::generate_distance_image(std::vector<bool> &visited)
{
    static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    int start_point = 0;
    int first_one = 0;
    
    for (int i = 1; i < height - 1; ++i)
        for (int j = 1; j < width - 1; ++j){
            int pointer = i * width + j;
            if (pixels[pointer] == 255) {
                if (first_one == 0)
                    first_one = pointer;
                int ngh = 0;
                for (int dir = 0; dir < 8; ++dir) {
                    int np = pointer + dy[dir] * width + dx[dir];
                    if (pixels[np] == 255)
                        ngh++;
                }
                if (ngh == 1) {
                    start_point = pointer;
                    goto done;
                }
            }
        }
    start_point = first_one;

done:

    std::queue<int> q;
    Image result(width, height);

    q.push(start_point);
    visited[start_point] = true;
    int p;

    while (!q.empty()) {
        p = q.front();
        q.pop();

        for (int dir = 0; dir < 8; ++dir) {
            int np = p + dy[dir] * width + dx[dir];
            if (pixels[np] == 255 && !visited[np]) {
                visited[np] = true;
                result[np] = result[p] + 1;
                q.push(np);
            }
        }
    }

    result.max = result[p];
    result.min = 0;

    std::cout << "Distanta de la (" << start_point % width << ", " <<
        start_point / width << ") la (" << p % width << ", " << p / width <<
        ") este de " << result[p] << " pixeli. " << std::endl;

    return result;
}