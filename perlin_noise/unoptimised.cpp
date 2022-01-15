// Copyright 2015 @aeiuo, from habr.com. All rights reserved.
#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <cmath>

#define N 3000

int *permutationTable;

float* GetPseudoRandomGradientVector(int x, int y){
    int v = (int)(((x * 1836311903) ^ (y * 2971215073) + 4807526976) & 1023);
    v = permutationTable[v] & 3;

    switch (v){
        case 0:  return new float[]{  1, 0 };
        case 1:  return new float[]{ -1, 0 };
        case 2:  return new float[]{  0, 1 };
        default: return new float[]{  0,-1 };
    }
}

float QunticCurve(float t){
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float Lerp(float a, float b, float t){
    return a + (b - a) * t;
}

float Dot(const float* a, const float* b){
    return a[0] * b[0] + a[1] * b[1];
}

float Noise(float fx, float fy){
    int left = std::floor(fx);
    int top  = std::floor(fy);
    float pointInQuadX = fx - left;
    float pointInQuadY = fy - top;

    float* topLeftGradient     = GetPseudoRandomGradientVector(left,   top  );
    float* topRightGradient    = GetPseudoRandomGradientVector(left+1, top  );
    float* bottomLeftGradient  = GetPseudoRandomGradientVector(left,   top+1);
    float* bottomRightGradient = GetPseudoRandomGradientVector(left+1, top+1);

    auto distanceToTopLeft     = new float[]{ pointInQuadX,   pointInQuadY   };
    auto distanceToTopRight    = new float[]{ pointInQuadX-1, pointInQuadY   };
    auto distanceToBottomLeft  = new float[]{ pointInQuadX,   pointInQuadY-1 };
    auto distanceToBottomRight = new float[]{ pointInQuadX-1, pointInQuadY-1 };

    float tx1 = Dot(distanceToTopLeft,     topLeftGradient);
    float tx2 = Dot(distanceToTopRight,    topRightGradient);
    float bx1 = Dot(distanceToBottomLeft,  bottomLeftGradient);
    float bx2 = Dot(distanceToBottomRight, bottomRightGradient);

    pointInQuadX = QunticCurve(pointInQuadX);
    pointInQuadY = QunticCurve(pointInQuadY);

    float tx = Lerp(tx1, tx2, pointInQuadX);
    float bx = Lerp(bx1, bx2, pointInQuadX);
    float tb = Lerp(tx, bx, pointInQuadY);

    return tb;
}

float Noise(float fx, float fy, int octaves, float persistence = 0.5f){
    float amplitude = 1;
    float max = 0;
    float result = 0;
    fx *= 0.01; fy *= 0.01;

    while(octaves-- > 0){
        result += Noise(fx, fy) * amplitude;
        max += amplitude;
        amplitude *= persistence;
        fx *= 2;
        fy *= 2;
    }
    return result/max;
}


int main(int argc, char **argv){
    std::random_device rd;
    std::mt19937 mersenne(0);
    permutationTable = new int[1024];
    for(int i = 0; i < 1024; i++) {
        permutationTable[i] = mersenne();
    }

    auto result = new float[N][N];
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            result[i][j] = Noise(i+0.5f, j+0.5f, 8);
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    std::cout << result[0][0] << ' ' << result[0][1] << ' ' << result[1][0] << std::endl;
    return 0;
}
