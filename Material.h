#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "defs.h"
#include "Eigen/Dense"

enum MaterialType{Normal, Mirror, Conductor, Dielectric};

class Material
{
public:
	int id;
	int phongExp = 0;
	bool isRough;
	MaterialType type;

	Eigen::Vector3f ambientRef;
	Eigen::Vector3f diffuseRef;
	Eigen::Vector3f specularRef;
	Eigen::Vector3f mirrorRef;

	float refractionIndex;
	float absorptionIndex;
	float roughness;
	Eigen::Vector3f absorptionCoefficient;

	Material(void);

private:

};

#endif
