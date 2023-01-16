#pragma once


#ifndef OBJECT_H
#define OBJECT_H
#include"mesh.hpp"
#include"opengl.hpp"
#include<iostream>
#include<assimp/Importer.hpp>
#include"component.hpp"
#include"transform.hpp"
#include<thread>
#include<functional>
#include<map>

class Object;

std::shared_ptr<Object> CreateObject();
std::shared_ptr<Object> CreateObject(const std::string& path);
std::shared_ptr<Object> CopyObject(std::shared_ptr<Object>);

class Object {
private:
    void loadModel(const std::string &path);
    void processNode(aiNode* node, const aiScene* scene);
    MeshTriangle* processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<std::shared_ptr<MeshTriangle>> meshes;
    std::vector<Component*> components;
    bool active;
    bool createFinish = false;

public:
    explicit Object(const std::string& filepath) :active(true) {
        AddComponent<Transform>();
        /*std::thread t(std::bind(&Object::loadModel,this, filepath, genBVH));
        t.join();*/
        loadModel(filepath);
    }

    explicit Object() :active(true) { AddComponent<Transform>(); }

    Object(Object& o) {
#ifdef _DEBUG
        std::cout << "Copy obj\n";
#endif // _DEBUG

        for (int i = 0; i < o.meshes.size(); i++) {
            MeshTriangle *mesh = new MeshTriangle();
            *mesh = *o.meshes.at(i);
            meshes.push_back(std::shared_ptr<MeshTriangle>(mesh));
        }
        for (int i = 0; i < o.components.size(); i++) {
            Component* com = new Component(*o.components[i]);
            components.push_back(com);
        }
        active = o.active;
        isStatic = o.isStatic;
        
    }

    Object(Object&& o) noexcept {
        meshes.swap(o.meshes);
        components.swap(o.components);
        active = o.active;
        isStatic = o.isStatic;
    }

    ~Object() {
        for (auto& com : components)
            delete com;
    }

    Object& operator=(Object& o) {
        meshes = o.meshes;
        for (auto& c : o.components) {
            auto tmp = c;
            components.push_back(c);
        }
            
        active = o.active;
        return *this;
    }

    void draw(int j) const &;

    void draw(Shader* s) const&;

    void drawGBuffer() const&;

    void drawInstance(unsigned) const&;

    template<typename T>
    T* GetComponent(){
        for (int j = 0; j < components.size();++j) {
            //Component &i = components.at(j);
            if (dynamic_cast<T*>(components[j]) != nullptr)
                return dynamic_cast<T*>(components[j]);
        }
        return NULL;
    }

    std::vector<Component*> GetComponents() const { return components; }

    template<class T>
    T* AddComponent() {
        T *t=new T();
        t->object=this;
        //std::shared_ptr<T> tmp(t);
        components.push_back(t);
        return t;
    }

    void AddComponent(Component* c) {
        components.push_back(c);
    }

    void SetShader(int index, std::shared_ptr<Shader> s,RenderPass p=Forward) {
        if(index>0||index<meshes.size())
            meshes.at(index)->SetShader(std::shared_ptr<Shader>(s),p);
        if (index == -1)
            for (auto& i : meshes)
                i->SetShader(std::shared_ptr<Shader>(s),p);
    }

    std::shared_ptr<Shader> GetShader(int index, RenderPass p = Forward) const{
        if (index >= 0 && index < GetMeshLength()) {
            if(p==Forward)
                return meshes.at(index)->forwardShader;

            else if(p==Deffered)
                return meshes.at(index)->defferedShader;
        }
            
        return NULL;
    }

    int GetMeshLength() const { return meshes.size(); }

    void SetActive(bool ac) {
        active = ac;
    }

    bool IsActive() const { return active; }

    void LoadLightMapData() {
        for (auto& m : meshes)
            m->LoadLightMapData();
    }

    void buildKdTree() {
        for (auto& m : meshes) {
            m->buildKdTree();
        }
    }

    void buildBVH() {
        for (auto& m : meshes) {
            Vector3f posOffset = GetComponent<Transform>()->GetPosition();
            m->buildBVH();
        }
    }

    bool isStatic;
    friend class Component;
    friend class Scene;
    friend std::shared_ptr<Object> CreateObject();
    friend std::shared_ptr<Object> CreateObject(const std::string& path);
};

#endif // !OBJECT_H
