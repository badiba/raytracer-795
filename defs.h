#ifndef _DEFS_H_
#define _DEFS_H_

#include "Eigen/Dense"

class Scene;

typedef struct ReturnVal
{
    Eigen::Vector3f point;
    Eigen::Vector3f normal;
    bool full = false;
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
