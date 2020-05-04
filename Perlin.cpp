#include <iostream>
#include "Perlin.h"

Perlin::Perlin() {
    table = new Eigen::Vector3f [16];
    table[0] = {1,1,0};
    table[1] = {-1,1,0};
    table[2] = {1,-1,0};
    table[3] = {-1,-1,0};
    table[4] = {1,0,1};
    table[5] = {-1,0,1};
    table[6] = {1,0,-1};
    table[7] = {-1,0,-1};
    table[8] = {0,1,1};
    table[9] = {0,-1,1};
    table[10] = {0,1,-1};
    table[11] = {0,-1,-1};
    table[12] = {1,1,0};
    table[13] = {-1,1,0};
    table[14] = {0,-1,1};
    table[15] = {0,-1,-1};

    epsilon = 0.001;
    shuffled = new int[16]{12,7,15,6,11,0,4,9,13,3,14,8,2,5,1,10};
}

float Perlin::Weight(float x) {
    x = abs(x);
    return ((-6)*pow(x,5)) + (15*pow(x,4)) - (10*pow(x,3)) + 1;
}

float Perlin::ComputeWeight(Eigen::Vector3f& v){
    return Weight(v[0])*Weight(v[1])*Weight(v[2]);
}

Eigen::Vector3f Perlin::GetGradient(Eigen::Vector3f &p, float scale, NoiseConversion nc) {
    Eigen::Vector3f xEpsilon = p;
    xEpsilon[0] += epsilon;
    Eigen::Vector3f yEpsilon = p;
    yEpsilon[1] += epsilon;
    Eigen::Vector3f zEpsilon = p;
    zEpsilon[2] += epsilon;

    float originalValue = ComputePerlin(p,scale, nc);
    float x = (ComputePerlin(xEpsilon,scale, nc) - originalValue) / epsilon;
    float y = (ComputePerlin(yEpsilon,scale, nc) - originalValue) / epsilon;
    float z = (ComputePerlin(zEpsilon,scale, nc) - originalValue) / epsilon;

    return Eigen::Vector3f{x,y,z};
}

float Perlin::ComputePerlin(Eigen::Vector3f &p, float scale, NoiseConversion nc) {
    Eigen::Vector3f point = p * scale;

    int index_i = (int)floor(point[0]);
    int index_j = (int)floor(point[1]);
    int index_k = (int)floor(point[2]);
    float value = 0;

    Eigen::Vector3i latticePoint;
    Eigen::Vector3f gradient;
    Eigen::Vector3f latticeToPoint;

    for (int i = 0; i < 2; i++){
        for (int j = 0; j < 2; j++){
            for (int k = 0; k < 2; k++){
                latticePoint = {index_i+i, index_j+j, index_k+k};
                gradient = GetGradientVector(latticePoint);
                latticeToPoint = {(float)latticePoint[0], (float)latticePoint[1], (float)latticePoint[2]};
                latticeToPoint = point - latticeToPoint;
                value += gradient.dot(latticeToPoint)*ComputeWeight(latticeToPoint);
            }
        }
    }

    if (nc == NCLinear){
        value = (value + 1) * 0.5f;
    }
    else if (nc == Absval){
        value = abs(value);
    }

    return value;
}

Eigen::Vector3f Perlin::GetGradientVector(Eigen::Vector3i& lp){
    int index = GetFromShuffled(lp[0] + GetFromShuffled(lp[1] + GetFromShuffled(lp[2])));
    return table[index];
}

int Perlin::GetFromShuffled(int i) {
    int index = i % 16;
    if (index < 0){
        index += 16;
    }
    return shuffled[index];
}