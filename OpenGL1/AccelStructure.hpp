#pragma once


#ifndef ACCEL_STRUCTURE_H
#define ACCEL_STRUCTURE_H
#include"intersection.hpp"

class AccelStructrue {
public:

	virtual Intersection Intersect(const Ray& ray) const { return {}; }
	virtual void Sample(Intersection& pos, float& pdf) {}
	virtual bool Intersect(const Ray& r, Intersection* inter) { return false; }
	virtual Bounds3 GetBounds() const { return {}; }
};

#endif // !ACCEL_STRUCTURE_H
