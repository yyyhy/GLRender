#pragma once


#ifndef SCENE_H
#define SCENE_H
#include"object.hpp"
#include "camera.hpp"
#include"light.hpp"
#include"commons.hpp"
#include"photon_mapping.hpp"

class Render;
class Scene;

class Scene {
private:
	std::vector<std::shared_ptr<Object>> objects;
	std::vector<Light*> lights;
	Camera* mainCamera;
	unsigned skyBuffer;
	Shader* sky;
	BVH* bvh;
	unsigned skyVAO;
	PhotonMapping pm;
public:

	~Scene() {
		delete sky;
		delete bvh;
	}

	Scene() :skyBuffer(0) {
		sky =new Shader("shaders/sky.vs", "shaders/sky.fs");
		sky->setInt("skybox", 0);
		glGenVertexArrays(1, &skyVAO);
		glBindVertexArray(skyVAO);
		unsigned VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), &boxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}

	void SetMainCamera(Camera* c) {
		mainCamera = c;
	}

	void AddObject(std::shared_ptr<Object> o) {
		objects.push_back(std::shared_ptr<Object>(o));
		if (mainCamera == NULL) {
			auto c = o->GetComponent<Camera>();
			if (c) {
				mainCamera = c;
			}
		}
	}

	void AddLight(Light* i) {
		lights.push_back(i);
	}

	int GetObjectsLength() const { return objects.size(); }

	void Update() {
		for (auto& o : objects) {
			if (o) {
				for (auto& c : o->GetComponents())
					c->Update();
			}
		}
	}

	unsigned GetSkyBox() const { return skyBuffer; }

	void SetSkyBox(unsigned box) { glDeleteTextures(1, &skyBuffer); skyBuffer = box; }

	void SetSkyBox(const std::vector<std::string>& faces) {
		glGenTextures(1, &skyBuffer);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBuffer);
		int width, height, nrChannels;

		for (int i = 0; i < 6; i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	friend class Render;

	Camera* const GetMainCamera() const { return mainCamera; }

	Intersection intersect(const Ray& ray) const {
		return this->bvh->Intersect(ray);
	}

	void buildBVH() {
		printf(" - Generating scene's BVH...\n\n");
		std::vector<Shape*> meshs;
		for (auto& i : objects) {
			for (int j = 0; j < i->GetMeshLength(); j++) {
				auto m = i->meshs[j];
				meshs.push_back(m.get());
			}
		}
		this->bvh = new BVH(meshs);
	}

	Vector3d castRay(const Ray& ray, int depth) const {
		return Vector3d(0, 0, 0);
	}
	
	void mappingPhotons(Light* l) {
		pm.mappingPhotons(bvh, l);
	}

	void printPhotons() const{
		pm.print();
	}

	void savePhotons() const {
		pm.saveFile();
	}

	void readPhotons() {
		pm.readFile();
	}

	void genLightMap(Object* o,int index) {
		if (index == -1) {
			for (auto& m : o->meshs)
				pm.genLightMap(m->vertices);
		}
		auto m = o->meshs.at(index);
		if(m!=NULL)
			pm.genLightMap(m->vertices);
	}

	const std::vector<Photon>& getPhotons() const { return pm.getPhotons(); }
};

#endif // SCENE_H
