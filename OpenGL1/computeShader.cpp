
#include"computeShader.hpp"

ComputeShader::~ComputeShader()
{
	glDeleteProgram(id);
}

void ComputeShader::Use() const& {
	glUseProgram(id);
}

void ComputeShader::SetInt(const std::string& name, int v) const& {
	glUniform1i(glGetUniformLocation(id, name.c_str()), v);
}

void ComputeShader::SetFloat(const std::string& name, float v) const&
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), v);
}

void ComputeShader::SetBool(const std::string& name, bool value) const&
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void ComputeShader::SetTexture(const std::string& name, const Texture& tex) &{
	textures[name] = tex;
}

void ComputeShader::SetTexture(const std::string& name, unsigned id)&
{
	Texture t;
	t.name = name;
	t.id = id;
	textures[name] = t;
}

void ComputeShader::SetVec3(const std::string& name, const glm::vec3& v) const&
{
	glUniform3fv(glGetUniformLocation(id, name.c_str()), 1,&v[0]);
}

void ComputeShader::SetVec2(const std::string& name, const glm::vec2& v) const&
{
	glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &v[0]);
}

void ComputeShader::SetBuffer(int index, const ComputeBuffer& ssbo) const&
{
	Use();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo.id);
}

void ComputeShader::SetBindingImage(int index, const Texture& tex) const&
{
	glUseProgram(id);
	glBindImageTexture(index, tex.id, 0, tex.type==GL_TEXTURE_3D? GL_TRUE:GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void ComputeShader::Dispath(int x, int y, int z) const& {
	int index = 0;
	glUseProgram(id);
	for (auto i = textures.begin(); i != textures.end(); i++, index++) {
		SetInt(i->first, index);
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(i->second.type, i->second.id);
		
	}
	
	glDispatchCompute(x, y, z);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}