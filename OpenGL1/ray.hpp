#pragma once

#ifndef RAY_H
#define RAY_H

#include"glm.hpp"


struct Ray
{
    Vector3d origin;
    Vector3d direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max;

    Ray(const Vector3d& ori, const Vector3d& dir, const double _t = 0.0) : origin(ori), direction(dir), t(_t) {
        direction_inv = Vector3d(1. / direction.x, 1. / direction.y, 1. / direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();

    }

    Vector3d operator()(double t) const { return origin + direction * t; }
};


#endif // !RAY_H
