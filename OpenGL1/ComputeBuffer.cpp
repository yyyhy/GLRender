#include "ComputeBuffer.hpp"

ComputeBuffer::ComputeBuffer(unsigned size, unsigned cnt):size(size),count(cnt)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, size * cnt, nullptr, GL_DYNAMIC_DRAW);

}

ComputeBuffer::~ComputeBuffer()
{
	glDeleteBuffers(1, &id);
}

void ComputeBuffer::SetData(void* data) const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size*count, data, GL_DYNAMIC_DRAW);
}

void* ComputeBuffer::ReadData() const
{	
	void* data;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	return data;
}
