#include "stdafx.h"
#include "Gaussian.h"

std::vector<double> generate_gaussian_kernel(double sigma)
{
    int size = (int) (3 * sigma + 0.5);
    if (size == 0)
        size = 1;
    std::vector<double> result(2 * size + 1);
    
    double exp_denom = - 2 * sigma * sigma;

    for (int i = 0; i <= size; ++i)
        result[size - i] = result[i + size] = exp((double)i * i / exp_denom);

    return result;
}

double calc_kernel_denom(const std::vector<double> &kernel)
{
    double sum = 0;
    //double gauss_denom = sigma * sqrt(2 * M_PI);
    //sum = std::accumulate(kernel.begin(), kernel.end(), 0);
    for (double e : kernel)
        sum += e;
    return sum;
}