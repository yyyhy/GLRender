#pragma once


#ifndef PHOTON_H
#define PHOTON_H
#include"glm.hpp"

struct Photon
{
	glm::vec3 positon;
	glm::vec3 dir;
	glm::vec3 color;
	float times=0;
	Photon() = default;

	Photon& operator=(const Photon& p){
		this->positon = p.positon;
		this->dir = p.dir;
		this->color = p.color;
		this->times = p.times;
		return *this;
	}

};

#endif // !PHOTON_H
