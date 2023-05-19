

#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H
#include<vector>
#include"RenderPipeline.hpp"

class Render;

class RenderManager {
private:
	std::vector<RenderPipeline*> renders;
	RenderManager(RenderManager&) {}
	RenderManager(RenderManager&&) noexcept {}

public:
	RenderManager() {}
	void RegisterRender(RenderPipeline* render) {
		renders.push_back(render);
	}

	void AutoReleaseRender() {

	}
	
};

static RenderManager RenderManagerInstance;
#endif // !RENDER_MANAGER_H
