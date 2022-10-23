#pragma once


#ifndef ACCEL_STRUCTURE_H
#define ACCEL_STRUCTURE_H

class AccelStructrue {
public:

	virtual Intersection Intersect(const Ray& ray) const { return {}; }
	virtual void Sample(Intersection& pos, float& pdf) {}
	virtual bool intersect(const Ray& r, Intersection* inter) { return false; }
	virtual Bounds3 getBounds() const { return {}; }
};

#endif // !ACCEL_STRUCTURE_H
