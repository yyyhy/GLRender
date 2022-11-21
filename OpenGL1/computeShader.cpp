
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
	Use();

	for (auto tex = textures.begin(); tex != textures.end(); tex++,++index) {
		SetInt(tex->first, index);
		glActiveTexture(GL_TEXTURE0 + index);
		if (tex->second.type == GL_TEXTURE_2D)
			glBindTexture(GL_TEXTURE_2D, tex->second.id);
	}

	glDispatchCompute(x, y, z);
}