// Copyright 2015 @aeiuo, from habr.com. All rights reserved.
#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <cmath>

#define N 3000

int *permutationTable;

#pragma omp declare simd uniform(v) notinbranch
float get_random_gradient0(int v){
    switch(v){
        case 0:  return 1;
        case 1:  return -1;
        case 2:  return 0;
        default: return 0;
    }
}

#pragma omp declare simd uniform(v) notinbranch
float get_random_gradient1(int v){
    switch(v){
        case 0:  return 0;
        case 1:  return 0;
        case 2:  return 1;
        default: return -1;
    }
}

#pragma omp declare simd notinbranch
float quantic_function(float t){
    return t * t * t * (t * (t * 6 - 15) + 10);
}


#pragma omp declare simd notinbranch
float linear_interpolation(float a, float b, float t){
    return a + (b - a) * t;
}

#pragma omp declare simd notinbranch
float dot_product(float a0, float a1, float b0, float b1){
    return a0 * b0 + a1 * b1;
}

#pragma omp declare simd notinbranch
float generate_noise(float fx, float fy){
    int left = std::floor(fx);
    int top  = std::floor(fy);
    float point_x = fx - left;
    float point_y = fy - top;

    int top_left_v = permutationTable[(int)(((left * 1836311903) ^ (top * 2971215073) + 4807526976) & 1023)] & 3;
    int top_right_v = permutationTable[(int)((((left+1) * 1836311903) ^ (top * 2971215073) + 4807526976) & 1023)] & 3;
    int bottom_left_v = permutationTable[(int)(((left * 1836311903) ^ ((top+1) * 2971215073) + 4807526976) & 1023)] & 3;
    int bottom_right_v = permutationTable[(int)((((left+1) * 1836311903) ^ ((top+1) * 2971215073) + 4807526976) & 1023)] & 3;

    float topLeftGradient_left = get_random_gradient0(top_left_v);
    float topLeftGradient_right = get_random_gradient1(top_left_v);
    float tx1 = dot_product(point_x, point_y, topLeftGradient_left, topLeftGradient_right);

    float topRightGradient_left = get_random_gradient0(top_right_v);
    float topRightGradient_right = get_random_gradient1(top_right_v);
    float tx2 = dot_product(point_x-1, point_y, topRightGradient_left, topRightGradient_right);

    float bottomLeftGradient_left = get_random_gradient0(bottom_left_v);
    float bottomLeftGradient_right = get_random_gradient1(bottom_left_v);
    float bx1 = dot_product(point_x,   point_y-1, bottomLeftGradient_left, bottomLeftGradient_right);

    float bottomRightGradient_left = get_random_gradient0(bottom_right_v);
    float bottomRightGradient_right = get_random_gradient1(bottom_right_v);
    float bx2 = dot_product(point_x-1, point_y-1, bottomRightGradient_left, bottomRightGradient_right);

    point_x = quantic_function(point_x);
    float tx = linear_interpolation(tx1, tx2, point_x);
    float bx = linear_interpolation(bx1, bx2, point_x);

    point_y = quantic_function(point_y);
    float tb = linear_interpolation(tx, bx, point_y);

    return tb;
}

#pragma omp declare simd uniform(octaves) notinbranch
float get_noise(float fx, float fy, int octaves, float persistence = 0.5f){
    float max = 0;
    float result = 0;
    float t1, t2;
    fx *= 0.01; fy *= 0.01;

    for(int i = 0; i < octaves; i++){
        max += pow(persistence, i);
        t1 = fx * pow(2, i);
        t2 = fy * pow(2, i);
        result += generate_noise(t1, t2) * pow(persistence, i);
    }

    return result/max;
}


int main(int argc, char **argv) {
    std::random_device rd;
    std::mt19937 mersenne(5340);
    permutationTable = new int[1024];
    for (int i = 0; i < 1024; i++) {
        permutationTable[i] = mersenne();
    }

    auto result = new float[N][N];

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

#pragma omp parallel for simd collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            result[i][j] = get_noise(i + 0.5f, j + 0.5f, 8);
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << result[0][0] << ' ' << result[0][1] << ' ' << result[1][0] << std::endl;
    return 0;
}