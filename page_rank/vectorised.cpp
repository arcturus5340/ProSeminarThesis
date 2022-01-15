// Copyright 2011 Allan Cruse, University of San Francisco. All rights reserved.
#include <cstring>
#include <random>
#include <iostream>
#include <chrono>

#define N_NODES  100 // number of nodes

typedef float Matrix[ N_NODES ][ N_NODES ];

Matrix digraph;


int main(int argc, char **argv){
    std::random_device rd;
    std::mt19937 mersenne(0);

    for(int i = 0; i < N_NODES; i++){
        for(int j = 0; j < N_NODES; j++){
            digraph[i][j] = float(mersenne() % 2);
        }
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    // compute entries for the corresponding transition matrix
    for (int i = 0; i < N_NODES; i++){
        float rowsum = 0;
        #pragma omp simd reduction(+: rowsum)
        for(int j = 0; j < N_NODES; j++){
            rowsum += digraph[i][j];
        }
        if (rowsum > 0)
            #pragma omp simd
            for (int j = 0; j < N_NODES; j++)
                digraph[i][j] = digraph[i][j]/rowsum;
        else // A fixup for the "dangling node" problem
            #pragma omp simd
            for (int j = 0; j < N_NODES; j++)
                digraph[i][j] = 1.0f / N_NODES;
    }

    // recompute the transition matrix entries
    float alpha = 0.85;
    #pragma omp simd collapse(2)
    for (int i = 0; i < N_NODES; i++) {
        for (int j = 0; j < N_NODES; j++) {
            float entry = digraph[i][j];
            entry = (alpha * entry) + ((1.0f - alpha) / N_NODES);
            digraph[i][j] = entry;
        }
    }

    // initialize the current matrix power
    Matrix current = { 0 };
    #pragma omp simd collapse(2)
    for (int i = 0; i < N_NODES; i++)
        for (int j = 0; j < N_NODES; j++)
            current[i][j] = (i == j) ? 1.0f : 0.0f;

    //----------------------------------------------------
    // main loop to display the sequence of matrix powers
    //----------------------------------------------------
    do {
        // compute the next matrix power
        Matrix product = { 0 };
        for (int i = 0; i < N_NODES; i++)
            for (int j = 0; j < N_NODES; j++)
            {
                float sum = 0.0;
                for (int k = 0; k < N_NODES; k++)
                    sum += current[i][k] * digraph[k][j];
                product[i][j] = sum;
            }

        std::memcpy(current, product, sizeof(Matrix));

        // see if the stationary vector has been reached yet
        float diff, square_diff = 0.0f;
        #pragma omp simd collapse(2)
        for (int j = 0; j < N_NODES; j++) {
            for (int i = 1; i < N_NODES; i++) {
                diff = (current[i][j] - current[0][j]);
                square_diff += diff * diff;
            }
        }
        if (square_diff < 0.0000000000001f) break;
    }while (true);

    // display the stationary vector with the node-name letters
    float rank[N_NODES];
    #pragma omp simd
    for (int j = 0; j < N_NODES; j++){
        rank[j] = current[0][j];
    }

    // perform a bubble-sort on the stationary vector's components
    int i = 0, j = 1;
    do{
        if (rank[i] < rank[j]){
            std::swap(rank[ i ], rank[ j ]);
            i = 0; j = 1;
        }else{
            ++i;
            ++j;
        }
    }while (j < N_NODES);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " [µs]" << std::endl;

    std::cout << rank[1] << ' ' << rank[2] << ' ' << rank[3] << std::endl;
    return 0;
}