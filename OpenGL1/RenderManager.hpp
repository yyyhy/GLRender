

#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H
#include<vector>

class Render;

class RenderManager {
private:
	std::vector<Render*> renders;
	RenderManager(RenderManager&) {}
	RenderManager(RenderManager&&) noexcept {}

public:
	RenderManager() {}
	void RegisterRender(Render* render) {
		renders.push_back(render);
	}

	void AutoReleaseRender() {

	}
	
};

static RenderManager RenderManagerInstance;
#endif // !RENDER_MANAGER_H
