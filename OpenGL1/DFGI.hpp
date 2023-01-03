#pragma once


#ifndef DFGI_H
#define DFGI_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"

class DFGI : public PostProcess{
public:


	DFGI() :PostProcess(), DFGIFirstRaySparseCS("shaders/cs/dfgi/DFGIFirstRaySparse.compute") { std::cout << "DFGI init\n"; }

	void excute() override;

	//First ray sparse data
	ComputeShader DFGIFirstRaySparseCS;
	Texture2D RSMAlbedoFlag;
	Texture2D RSMNormalRoughness;
	Texture2D RSMPositionMetallic;
	Texture3D gSDF;
	glm::vec3 GlobalSDFBoxMin;
	glm::vec3 GlobalSDFBoxMax;
	glm::ivec3 SceneGridsResolution;
	glm::vec3 LightFlux;
	
};

#endif // !DFGI_H
