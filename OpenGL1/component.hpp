#pragma once


#ifndef COMPONENT_H
#define COMPONENT_H
#include<iostream>
class Object;

class Component {
public:
    
    Component(Object* fa = NULL, bool en = true) { object = fa; SetEnable(en); Start(); }

    Component(Component& com) {
        object = com.object;
        enable = com.enable;
        name = com.name;
    }

    ~Component() {
#ifdef _DEBUG
        std::cout << "component lose :"<<name<<"\n";
#endif // DEBUG
        Exit();
    }

    virtual void OnEnable() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Exit() {}

    void SetEnable(bool en) { enable = en; if (en) OnEnable(); }

    bool IsEnable() const { return enable; }

    std::string name;

    Object* object;
    bool enable=true;
};


class Transform;
#endif // !COMPONENT_H

