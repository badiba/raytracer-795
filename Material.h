#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "defs.h"
#include "Eigen/Dense"

class Material
{
public:
	int id;
	int phongExp = 0;
	Eigen::Vector3f ambientRef;
	Eigen::Vector3f diffuseRef;
	Eigen::Vector3f specularRef;
	Eigen::Vector3f mirrorRef;

	Material(void);

private:

};

#endif