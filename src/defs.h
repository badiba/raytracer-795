#ifndef _DEFS_H_
#define _DEFS_H_

#include "Eigen/Dense"

class Scene;

enum DecalMode{ReplaceKd, BlendKd, BumpNormal, ReplaceNormal, ReplaceAll, ReplaceBackground, NoDecal};
enum Interpolation{NN, Bilinear};
enum TextureType{ImageTexture, PerlinTexture};
enum NoiseConversion{Absval, NCLinear, NoConversion};

typedef struct ReturnVal
{
    Eigen::Vector3f point;
    Eigen::Vector3f normal;
    bool full = false;
    int matIndex;
    DecalMode dm;
    Eigen::Vector3f textureColor;
    float textureNormalizer;
} ReturnVal;

typedef struct BBox
{
	Eigen::Vector3f minPoint;
	Eigen::Vector3f maxPoint;

	int startIndex;
	int endIndex;
} BBox;

// The global variable through which you can access the scene data
extern Scene* pScene;

#endif
