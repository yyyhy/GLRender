#pragma once

#ifndef BOUND_H
#define BOUND_H
#include"glm.hpp"
#include <array>
#include"ray.hpp"
#define EPSILON 0.01

class Bounds3{
public:
    Vector3d pMin, pMax; // two points to specify the bounding box

    Bounds3(){
        constexpr double minNum = std::numeric_limits<double>::lowest();
        constexpr double maxNum = std::numeric_limits<double>::max();
        pMax = Vector3d(minNum, minNum, minNum);
        pMin = Vector3d(maxNum, maxNum, maxNum);
    }

    Bounds3(const Vector3d p) : pMin(p), pMax(p) {}

    Bounds3(const Vector3d p1, const Vector3d p2){
        pMin = Vector3d(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
        pMax = Vector3d(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
    }

    Vector3d Diagonal() const { return pMax - pMin; }

    int maxExtent() const{
        Vector3d d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    double SurfaceArea() const{
        Vector3d d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    Vector3d Centroid() { return 0.5 * pMin + 0.5 * pMax; }

    Bounds3 Intersect(const Bounds3& b) {
        return Bounds3(Vector3d(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y),
            fmax(pMin.z, b.pMin.z)),
            Vector3d(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y),
                fmin(pMax.z, b.pMax.z)));
    }

    Vector3d Offset(const Vector3d& p) const{
        Vector3d o = p - pMin;
        if (pMax.x > pMin.x)
            o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y)
            o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z)
            o.z /= pMax.z - pMin.z;
        return o;
    }

    bool Overlaps(const Bounds3& b1, const Bounds3& b2){
        bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
        bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
        bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
        return (x && y && z);
    }

    bool Inside(const Vector3d& p){
        return (p.x >= pMin.x && p.x <= pMax.x && p.y >= pMin.y &&
            p.y <= pMax.y && p.z >= pMin.z && p.z <= pMax.z);
    }

    const Vector3d& operator[](int i) const{
        return (i == 0) ? pMin : pMax;
    }

    bool IntersectP(const Ray& ray, const Vector3d& invDir) const;

    bool Intersect(const Ray& ray, float* tMin, float* tMax) const;
};

inline bool Bounds3::IntersectP(const Ray& ray, const Vector3d& invDir) const
{
    Vector3d o = ray.origin;
    Vector3d p1 = pMin;
    Vector3d p2 = pMax;
    float tEnter = -99999;
    float tExit = 99999;
    float t1, t2;
    t1 = (p1.x - o.x) * invDir.x;
    t2 = (p2.x - o.x) * invDir.x;
    tEnter = std::max({ std::min(t1,t2),tEnter });
    tExit = std::min({ std::max(t1,t2),tExit });
    t1 = (p1.y - o.y) * invDir.y;
    t2 = (p2.y - o.y) * invDir.y;
    tEnter = std::max({ std::min(t1,t2),tEnter });
    tExit = std::min({ std::max(t1,t2),tExit });
    t1 = (p1.z - o.z) * invDir.z;
    t2 = (p2.z - o.z) * invDir.z;
    tEnter = std::max({ std::min(t1,t2),tEnter });
    tExit = std::min({ std::max(t1,t2),tExit });

    return tExit >= -EPSILON && tEnter - tExit <= EPSILON;
}

inline bool Bounds3::Intersect(const Ray& ray, float* tMin, float* tMax) const
{
    Vector3f invDir(ray.direction_inv);
    Vector3d o = ray.origin;
    Vector3d p1 = pMin;
    Vector3d p2 = pMax;
    float tEnter = -99999;
    float tExit = 99999;
    float t1, t2;
    t1 = (p1.x - o.x) * invDir.x;
    t2 = (p2.x - o.x) * invDir.x;
    tEnter = std::max({ std::min(t1,t2),tEnter });
    tExit = std::min({ std::max(t1,t2),tExit });
    t1 = (p1.y - o.y) * invDir.y;
    t2 = (p2.y - o.y) * invDir.y;
    tEnter = std::max({ std::min(t1,t2),tEnter });
    tExit = std::min({ std::max(t1,t2),tExit });
    t1 = (p1.z - o.z) * invDir.z;
    t2 = (p2.z - o.z) * invDir.z;
    tEnter = std::max({ std::min(t1,t2),tEnter });
    tExit = std::min({ std::max(t1,t2),tExit });
    if (!(tExit >= -EPSILON && tEnter - tExit <= EPSILON))
        return false;

    *tMin = tEnter;
    *tMax = tExit;
    return true;
}

inline Vector3d Min(const Vector3d& v1, const Vector3d& v2) {
    Vector3d v;
    v.x = std::min(v1.x, v2.x);
    v.y = std::min(v1.y, v2.y);
    v.z = std::min(v1.z, v2.z);
    return v;
}

inline Vector3d Max(const Vector3d& v1, const Vector3d& v2) {
    Vector3d v;
    v.x = std::max(v1.x, v2.x);
    v.y = std::max(v1.y, v2.y);
    v.z = std::max(v1.z, v2.z);
    return v;
}

inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2)
{
    Bounds3 ret;
    ret.pMin = Min(b1.pMin, b2.pMin);
    ret.pMax = Max(b1.pMax, b2.pMax);
    return ret;
}

inline Bounds3 Union(const Bounds3& b, const Vector3d& p)
{
    Bounds3 ret;
    ret.pMin = Min(b.pMin, p);
    ret.pMax = Max(b.pMax, p);
    return ret;
}
#endif // !BOUND_H
