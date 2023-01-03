#pragma once


#ifndef COMPUTEBUFFER_H
#define COMPUTEBUFFER_H


#include"opengl.hpp"

class ComputeBuffer {
private:
	unsigned size;
	unsigned count;
	unsigned id;
public:
	ComputeBuffer(unsigned size, unsigned cnt);
	ComputeBuffer(ComputeBuffer&) = delete;
	ComputeBuffer(ComputeBuffer&&) = delete;

	void SetData(void*, unsigned) const;

	void ReadData(void*) const;
	
	friend class ComputeShader;
};
#endif // !COMPUTEBUFFER_H
