#include "DFGI.hpp"

void DFGI::excute()
{
	//
	if (RegenerateCounter >= RegenerateWeightFrequency) {
		DFFGIInitBufferCS.Use();
		DFFGIInitBufferCS.SetBuffer(0, DFGIWeightBuffer);
		DFFGIInitBufferCS.SetVec2("RSMSampleResolution", RSMSampleResolution);
		DFFGIInitBufferCS.Dispath(RSMSampleResolution.x, RSMSampleResolution.y, 1);
		RegenerateCounter = 0;
		CurrRSMSampleIndex = { -1,0 };
	}

	//
	if (FirstSparseCounter >= FirstSparseFrequency) {

		if (RegenerateCounter == 0) {
			CurrRSMSampleIndex.x++;
			if (CurrRSMSampleIndex.x >= RSMSampleResolution.x) {
				CurrRSMSampleIndex.y++;
				CurrRSMSampleIndex.x = 0;
				if (CurrRSMSampleIndex.y >= RSMSampleResolution.y) {
					RegenerateCounter = 1;
					weight = (unsigned*)DFGIWeightBuffer.ReadData();
					unsigned totol = 0;
					weightTable.clear();
					for (int y = 0; y < RSMSampleResolution.y; ++y) {
						for (int x = 0; x < RSMSampleResolution.x; ++x) {
							int index = y * RSMSampleResolution.x + x;
							if (weight[index] != 0) {
								weightTable[{totol,totol+weight[index]}] = glm::vec2(x, y);
								totol += weight[index];
							}
						}
					}
					totolWeight = totol;
					dis = std::uniform_int_distribution<int>(0, totolWeight);
					engine = std::default_random_engine(time(0));
				}
			}
		}
		if (RegenerateCounter != 0) {
			int random = dis(engine);
			auto it = std::find_if(weightTable.begin(), weightTable.end(), [&](const std::pair<RangePair, glm::vec2>& p) {
				return p.first.left <= random && p.first.right > random;
			});
			CurrRSMSampleIndex = it->second;
		}

		DFGIFirstRaySparseCS.Use();
		DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
		DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
		DFGIFirstRaySparseCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
		DFGIFirstRaySparseCS.SetVec3("LightFlux", LightFlux);
		DFGIFirstRaySparseCS.SetVec2("SampleOffset", sampleOffset);
		DFGIFirstRaySparseCS.SetVec3("LightDirection", LightDirection);
		DFGIFirstRaySparseCS.SetVec2("CurrRSMSampleIndex", CurrRSMSampleIndex);
		DFGIFirstRaySparseCS.SetVec2("RSMSampleResolution", RSMSampleResolution);
		DFGIFirstRaySparseCS.SetVec2("RSMSampleBrickSize", RSMSampleBrickSize);

		DFGIFirstRaySparseCS.SetBuffer(0, DFGIRaysBuffer);
		DFGIFirstRaySparseCS.SetBuffer(1, DFGIRayCounterBuffer);
		DFGIFirstRaySparseCS.SetBuffer(2, DFGIWeightBuffer);
		DFGIFirstRaySparseCS.SetTexture("GlobalSDF", gSDF);

		for (int i = 1; i < 2; ++i) {

			DFGIFirstRaySparseCS.SetTexture("RSMAlbedoFlag", RSMAlbedoFlag[i]);
			DFGIFirstRaySparseCS.SetTexture("RSMNormalRoughness", RSMNormalRoughness[i]);
			DFGIFirstRaySparseCS.SetTexture("RSMPositionMetallic", RSMPositionMetallic[i]);
			DFGIFirstRaySparseCS.SetTexture("RSMTangent", RSMTangent[i]);
			DFGIFirstRaySparseCS.Dispath(RSMSampleBrickSize.x, RSMSampleBrickSize.y, 1);
		}

		FirstSparseCounter = 0;
	}
	
	//
	DFGIRaysInjectGridCS.Use();
	DFGIRaysInjectGridCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIRaysInjectGridCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIRaysInjectGridCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIRaysInjectGridCS.SetBuffer(0, DFGIRaysBuffer);
	DFGIRaysInjectGridCS.SetBuffer(1, DFGIGridLightInfosBuffer);
	DFGIRaysInjectGridCS.SetBuffer(2, DFGIRayCounterBuffer);
	DFGIRaysInjectGridCS.Dispath(32, 32, 32);

	
	//
	DFGIApplyCS.Use();
	DFGIApplyCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIApplyCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIApplyCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIApplyCS.SetTexture("inputImage", GetInTexBuffer());
	DFGIApplyCS.SetTexture("gPositionRoughness", gPositionRoughness);
	DFGIApplyCS.SetTexture("gAlbedoMetallic", gAlbedoMetallic);
	DFGIApplyCS.SetTexture("gNormalDepth", gNormalDepth);
	DFGIApplyCS.SetTexture("gSDF", gSDF);
	DFGIApplyCS.SetBindingImage(1, DFGIResult);
	DFGIApplyCS.SetBuffer(2, DFGIGridLightInfosBuffer);
	DFGIApplyCS.SetBuffer(3, DFGIGridCanSparseBuffer);
	DFGIApplyCS.SetVec2("offset", sampleOffset);
	DFGIApplyCS.Dispath(width/ DFGI_DOWN_SAMPLE, height/ DFGI_DOWN_SAMPLE, 1);

	//
	DFGIBlurGBufferCS.Use();
	DFGIBlurGBufferCS.SetBindingImage(0, gAlbedoMetallic);
	DFGIBlurGBufferCS.SetBindingImage(1, BlurGBuffer);
	DFGIBlurGBufferCS.Dispath(width, height, 1);

	//
	if (MultBounceCounter >= MultBounceFrequency) {
		DFGIRayMultBounceCS.Use();
		DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
		DFGIRayMultBounceCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
		DFGIRayMultBounceCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
		DFGIRayMultBounceCS.SetTexture("gPositionRoughness", gPositionRoughness);
		DFGIRayMultBounceCS.SetTexture("gAlbedoMetallic", BlurGBuffer);
		DFGIRayMultBounceCS.SetTexture("gNormalDepth", gNormalDepth);
		DFGIRayMultBounceCS.SetTexture("gTangent", gTangent);
		DFGIRayMultBounceCS.SetTexture("GlobalSDF", gSDF);
		DFGIFirstRaySparseCS.SetBuffer(1, DFGIRaysBuffer);
		DFGIFirstRaySparseCS.SetBuffer(2, DFGIRayCounterBuffer);
		DFGIFirstRaySparseCS.SetBuffer(3, DFGIGridCanSparseBuffer);
		DFGIApplyCS.SetBindingImage(4, GetInTexBuffer());
		DFGIRayMultBounceCS.Dispath(32, 32, 32);
		MultBounceCounter = 0;
	}
	
	//
	DFGICompositeResultCS.Use();
	DFGICompositeResultCS.SetTexture("DFGIResult", DFGIResult);
	DFGICompositeResultCS.SetBindingImage(0, GetInTexBuffer());
	DFGICompositeResultCS.Dispath(width, height, 1);

	//
	DFGIRayLiveDownCS.Use();
	DFGIRayLiveDownCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIRayLiveDownCS.SetBuffer(0, DFGIRaysBuffer);
	//DFGIRayLiveDownCS.Dispath(32, 32, 32);

	sampleOffset = sampleOffset + glm::vec2(1,0);
	if (sampleOffset.x > 15) {
		sampleOffset.x = 0;
		sampleOffset.y++ ;
		if (sampleOffset.y > 15) {
			sampleOffset.y = 0;
		}
	}
	MultBounceCounter++;
	FirstSparseCounter++;
	if (RegenerateCounter != 0)
		RegenerateCounter++;
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
