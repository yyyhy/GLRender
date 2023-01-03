#include "ComputeBuffer.hpp"

ComputeBuffer::ComputeBuffer(unsigned size, unsigned cnt):size(size),count(cnt)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size * cnt, nullptr, GL_DYNAMIC_DRAW);

}

void ComputeBuffer::SetData(void* data,unsigned size) const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void ComputeBuffer::ReadData(void* data) const
{
}
