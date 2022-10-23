#pragma once


#ifndef BVN_H
#define BVH_H
#include"bound.hpp"
#include"shape.hpp"
#include"intersection.hpp"
#include<vector>
#include"AccelStructure.hpp"

struct BVHNode
{
    Bounds3 bounds;
    BVHNode* left;
    BVHNode* right;
    Shape* object;
    float area;
public:
    int splitAxis = 0, firstPrimOffset = 0, nPrimitives = 0;
    BVHNode() {
        bounds = Bounds3();
        left = nullptr; right = nullptr;
        object = nullptr;
    }

    ~BVHNode() {
        delete left;
        delete right;
    }

};

class BVH : public AccelStructrue{
public:

	BVH(std::vector<Shape*> objs);

    ~BVH() {
        delete root;
    }

	BVHNode* build(std::vector<Shape*> objs);
	
    Intersection Intersect(const Ray& ray) const;

    Intersection getIntersection(BVHNode* node, const Ray& ray)const;

    void Sample(Intersection& pos, float& pdf);

    Bounds3 getBounds() const override {
        return root->bounds;
    }

    BVHNode* root;
};


#endif // !BVN_H
