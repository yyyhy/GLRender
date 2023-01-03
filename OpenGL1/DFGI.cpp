#include "DFGI.hpp"

void DFGI::excute()
{
	DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMin", GlobalSDFBoxMin);
	DFGIFirstRaySparseCS.SetVec3("GlobalSDFBoxMax", GlobalSDFBoxMax);
	DFGIFirstRaySparseCS.SetVec3("SceneGridsResolution", glm::vec3(32, 32, 32));
	DFGIFirstRaySparseCS.SetVec3("LightFlux", glm::vec3(10, 5, 5));
}
