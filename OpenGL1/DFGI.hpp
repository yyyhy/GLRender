#pragma once


#ifndef DFGI_H
#define DFGI_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"
#include"DFGIRayStruct.hpp"


class DFGI : public PostProcess{
public:


	DFGI(int w, int h) :PostProcess(nullptr, nullptr, w, h, 0),
			DFGIFirstRaySparseCS("shaders/cs/dfgi/DFGIFirstRaySparse.comp"),
			DFGIRaysInjectGridCS("shaders/cs/dfgi/DFGIRaysInjectGrid.comp"),
			DFGIRaysBuffer(sizeof(DFGIRay), 32 * 32 * 32 * 16),
			DFGIRayCounterBuffer(sizeof(unsigned), 32 * 32 * 32),
			DFGIGridLightInfosBuffer(sizeof(DFGIGridLightInfo),32*32*32),
			DFGIApplyCS("shaders/cs/dfgi/DFGIApply.comp"),
			sampleOffset(0,0),
			DFGIRayMultBounceCS("shaders/cs/dfgi/DFGIRayMultBounce.comp"),
			DFGIGridCanSparseBuffer(sizeof(unsigned),32*32*32){
		DFGIRaysBuffer.SetData(new DFGIRay[32 * 32 * 32 * 16]);
		DFGIRayCounterBuffer.SetData(new unsigned[32 * 32 * 32] {0});
		DFGIGridLightInfosBuffer.SetData(new DFGIGridLightInfo[32 * 32 * 32]);
		DFGIGridCanSparseBuffer.SetData(new unsigned[32 * 32 * 32] {0});
		std::cout << "DFGI init\n";
	}

	void excute() override;

	void SendBufferToNext(PostProcess*)  override;

	void Debug();
	//First ray sparse data
	ComputeShader DFGIFirstRaySparseCS;
	ComputeBuffer DFGIRaysBuffer;
	ComputeBuffer DFGIRayCounterBuffer;
	Texture2D RSMAlbedoFlag[4];
	Texture2D RSMNormalRoughness[4];
	Texture2D RSMPositionMetallic[4];
	Texture3D gSDF;
	glm::vec3 GlobalSDFBoxMin;
	glm::vec3 GlobalSDFBoxMax;
	glm::ivec3 SceneGridsResolution;
	glm::vec3 LightFlux;
	glm::vec2 sampleOffset;

	//Compute grid light information
	ComputeShader DFGIRaysInjectGridCS;
	ComputeBuffer DFGIGridLightInfosBuffer;

	//Apply DFGI
	ComputeShader DFGIApplyCS;
	Texture2D gPositionRoughness;
	Texture2D gNormalDepth;
	Texture2D gAlbedoMetallic;
	ComputeBuffer DFGIGridCanSparseBuffer;

	//MultBounce
	ComputeShader DFGIRayMultBounceCS;

};

#endif // !DFGI_H
