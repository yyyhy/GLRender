#pragma once


#ifndef COMPUTEBUFFER_H
#define COMPUTEBUFFER_H


#include"opengl.hpp"

class ComputeBuffer {
private:
	unsigned size;
	unsigned long count;
	unsigned id;
public:
	ComputeBuffer(unsigned size, unsigned long cnt);
	ComputeBuffer(ComputeBuffer&) = delete;
	ComputeBuffer(ComputeBuffer&&) = delete;

	~ComputeBuffer();
	void SetData(void*) const;

	void* ReadData() const;
	
	friend class ComputeShader;
};
#endif // !COMPUTEBUFFER_H
