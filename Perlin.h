#ifndef _PERLIN_H_
#define _PERLIN_H_

#include "defs.h"
#include "math.h"

class Perlin {
public:
    Perlin();
    float ComputePerlin(Eigen::Vector3f& p, float scale, NoiseConversion nc);
    Eigen::Vector3f GetGradient(Eigen::Vector3f& p, float scale, NoiseConversion nc);
private:
    int GetFromShuffled(int i);
    Eigen::Vector3f GetGradientVector(Eigen::Vector3i& lp);
    Eigen::Vector3f* table;
    int* shuffled;
    float epsilon;
    float Weight(float x);
    float ComputeWeight(Eigen::Vector3f& v);
};


#endif
