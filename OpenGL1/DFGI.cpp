#include "DFGI.hpp"

void DFGI::excute()
{
	//
	DFGIFirstRaySparseCS.Use();
	DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIFirstRaySparseCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIFirstRaySparseCS.SetVec3("LightFlux", LightFlux);
	DFGIFirstRaySparseCS.SetVec2("SampleOffset", sampleOffset);

	DFGIFirstRaySparseCS.SetBuffer(0, DFGIRaysBuffer);
	DFGIFirstRaySparseCS.SetBuffer(1, DFGIRayCounterBuffer);
	DFGIFirstRaySparseCS.SetTexture("GlobalSDF", gSDF);

	for (int i = 0; i < 4; ++i) {
		
		DFGIFirstRaySparseCS.SetTexture("RSMAlbedoFlag", RSMAlbedoFlag[i]);
		DFGIFirstRaySparseCS.SetTexture("RSMNormalRoughness", RSMNormalRoughness[i]);
		DFGIFirstRaySparseCS.SetTexture("RSMPositionMetallic", RSMPositionMetallic[i]);

		DFGIFirstRaySparseCS.Dispath(width / 16, height / 16, 1);
	}

	//
	DFGIRaysInjectGridCS.Use();
	DFGIRaysInjectGridCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIRaysInjectGridCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIRaysInjectGridCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIRaysInjectGridCS.SetBuffer(0, DFGIRaysBuffer);
	DFGIRaysInjectGridCS.SetBuffer(1, DFGIGridLightInfosBuffer);

	DFGIRaysInjectGridCS.Dispath(32, 32, 32);

	
	//
	DFGIApplyCS.Use();
	DFGIApplyCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIApplyCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIApplyCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIApplyCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGIApplyCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGIApplyCS.SetTexture("gNormalDepth", gNormalDepth);
	DFGIApplyCS.SetTexture("gSDF", gSDF);
	DFGIApplyCS.SetBindingImage(1, GetInTexBuffer());
	DFGIApplyCS.SetBuffer(2, DFGIGridLightInfosBuffer);
	DFGIApplyCS.SetBuffer(3, DFGIGridCanSparseBuffer);
	DFGIApplyCS.SetVec2("offset", sampleOffset);
	DFGIApplyCS.Dispath(1600, 900, 1);

	//
	DFGIRayMultBounceCS.Use();
	DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIRayMultBounceCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIRayMultBounceCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGIRayMultBounceCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGIRayMultBounceCS.SetTexture("gNormalDepth", gNormalDepth);
	DFGIRayMultBounceCS.SetTexture("GlobalSDF", gSDF);
	DFGIFirstRaySparseCS.SetBuffer(1, DFGIRaysBuffer);
	DFGIFirstRaySparseCS.SetBuffer(2, DFGIRayCounterBuffer);
	DFGIFirstRaySparseCS.SetBuffer(3, DFGIGridCanSparseBuffer);
	DFGIApplyCS.SetBindingImage(4, GetInTexBuffer());
	DFGIRayMultBounceCS.Dispath(32, 32, 32);

	sampleOffset = sampleOffset + glm::vec2(1,0);
	if (sampleOffset.x > 15) {
		sampleOffset.x = 0;
		sampleOffset.y++ ;
		if (sampleOffset.y > 15) {
			sampleOffset.y = 0;
		}
	}
}

void DFGI::SendBufferToNext(PostProcess* p)
{
	p->SetInTexBuffer(GetInTexBuffer());
}

void DFGI::Debug()
{
	/*DFGIRay* rays = (DFGIRay*)DFGIRaysBuffer.ReadData();
	for (int i = 0; i < 4; ++i) {
		std::cout << rays[i].hitPosition.x << " "<<rays[i].direction.x<<"  ";
	}*/
	/*unsigned* cntrs = (unsigned*)DFGIRayCounterBuffer.ReadData();
	for (int i = 0; i < 4; ++i) {
		std::cout << cntrs[i] << " ";
	}*/

	DFGIGridLightInfo* infos = (DFGIGridLightInfo*)DFGIGridLightInfosBuffer.ReadData();
	for (int i = 0; i < 32 * 32 * 32; ++i) {
		if (infos[i].importanceDirection.x != 0|| infos[i].importanceDirection.y != 0|| infos[i].importanceDirection.z != 0) {
			std::cout << infos[i].importancePosition.x << " " << infos[i].importancePosition.y << " " << infos[i].importancePosition.z << "\n";
			return;
		}
	}
	std::cout << "\n";
}
