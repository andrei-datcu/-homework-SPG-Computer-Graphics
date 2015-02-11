#pragma once

#include <vector>
class Image
{
public:
    Image(unsigned char *from_corona, unsigned int width, unsigned int height);
    Image(unsigned int width, unsigned int height);
    void to_corona(unsigned char *data);
    void to_corona_red(unsigned char *data, std::vector<bool> &rc);
    void convolve (double *mask, unsigned mask_width, unsigned mask_height,
        double denom = 1.0);

    void addAbs(const Image& secondImage);
    void abs();
    void normalize(unsigned xborder = 0, unsigned yborder = 0);
    void clip();

    void smart_fill(int x, int y, int c);
    void thin();
    void thin_std();

    const unsigned int width, height;

    int& operator [](unsigned index);

    Image generate_distance_image(std::vector<bool> &visited);

private:
    int min, max;
    std::vector<int> pixels;
};

