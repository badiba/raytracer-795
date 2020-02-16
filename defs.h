#ifndef _DEFS_H_
#define _DEFS_H_

#include "Eigen/Dense"
using namespace Eigen;
class Scene;

typedef struct ReturnVal
{
	 Vector3f point;
	 Vector3f normal;
	 bool full = false;
} ReturnVal;

// The global variable through which you can access the scene data
extern Scene* pScene;

#endif
