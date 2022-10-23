

#ifndef SHAPE_H
#define SHAPE_H

#include "glm.hpp"
#include "bound.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Shape
{
public:
    Shape() {}
    virtual ~Shape() {}
    virtual bool intersect(const Ray& ray) = 0;
    virtual bool intersect(const Ray& ray, float&, uint32_t&) const = 0;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual void getSurfaceProperties(const Vector3d&, const Vector3d&, const uint32_t&, const Vector2d&, Vector3d&, Vector2d&) const = 0;
    virtual Vector3d evalDiffuseColor(const Vector2d&) const = 0;
    virtual Bounds3 getBounds() = 0;
    virtual float getArea() = 0;
    virtual void Sample(Intersection& pos, float& pdf) = 0;
};
#endif // !SHAPE_H
