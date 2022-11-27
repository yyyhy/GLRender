
#include"computeShader.hpp"

void ComputeShader::Use() const& {
	glUseProgram(id);
}

void ComputeShader::SetInt(const std::string& name, int v) const& {
	glUniform1i(glGetUniformLocation(id, name.c_str()), v);
}

void ComputeShader::SetTexture(const std::string& name, const Texture& tex) &{
	textures[name] = tex;
}

void ComputeShader::Dispath(int x, int y, int z) const& {
	int index = 0;
	glUseProgram(id);
	auto tex = textures.begin();
	/*tex++;
	glBindImageTexture(1, tex->second.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);*/

	glDispatchCompute(x, y, z);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}