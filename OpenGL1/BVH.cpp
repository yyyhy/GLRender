
#include"BVH.hpp"
#include<algorithm>


BVH::BVH(std::vector<Shape*> objs)
{
	root = build(objs);
}

BVHNode* BVH::build(std::vector<Shape*> objects)
{
    BVHNode* node = new BVHNode();
    int l = objects.size();
    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i) {
        auto box = objects[i];
        if (i == 0)
            bounds = objects[i]->getBounds();
        else
            bounds = Union(bounds, objects[i]->getBounds());
    }
        
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = objects[0]->getArea();
        return node;
    }
    else if (objects.size() == 2) {
        node->left = build(std::vector<Shape*>{ objects[0] });
        node->right = build(std::vector<Shape*>{ objects[1] });

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
            Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                    f2->getBounds().Centroid().x;
                });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                    f2->getBounds().Centroid().y;
                });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                    f2->getBounds().Centroid().z;
                });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Shape*>(beginning, middling);
        auto rightshapes = std::vector<Shape*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = build(leftshapes);
        node->right = build(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
    }

    return node;
}

Intersection BVH::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = getIntersection(root, ray);
    return isect;
}

Intersection BVH::getIntersection(BVHNode* node, const Ray& ray) const
{
    Intersection se1;
    Intersection se2;

    // TODO Traverse the BVH to find intersection
    std::array<int, 3> arr = { ray.direction.x > 0,ray.direction.x > 0,ray.direction.x > 0 };
    if (node->bounds.IntersectP(ray, ray.direction_inv)) {
        if (node->left == NULL && node->right == NULL)
            return node->object->getIntersection(ray);
        else {
            se1 = getIntersection(node->left, ray);
            se2 = getIntersection(node->right, ray);
        }
    }
    else
        return Intersection();

    return se1.distance < se2.distance ? se1 : se2;
}

void BVH::Sample(Intersection& pos, float& pdf)
{
}
