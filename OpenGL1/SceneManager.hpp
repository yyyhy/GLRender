#pragma once


#include"scene.hpp"

class SceneManager {

public:
	SceneManager() = default;
	void init(){}
	void LoadScene(unsigned index){}
	void SetScene(std::shared_ptr<Scene> s) { currScene = s; }
	std::shared_ptr<Scene> currScene;
};