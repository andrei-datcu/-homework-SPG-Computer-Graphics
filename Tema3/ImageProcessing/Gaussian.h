#pragma once

#include <vector>

std::vector<double> generate_gaussian_kernel(double sigma);
double calc_kernel_denom(const std::vector<double> &kernel);