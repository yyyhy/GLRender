#pragma once


#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include <glad/glad.h>
#include"glm.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include<map>
#include"texture.hpp"

class ComputeShader {
private:
	unsigned id;
	std::map<std::string, Texture> textures;

public:
	explicit ComputeShader(const char* path):id(0) {
		std::string code;

		std::ifstream shaderFile;
		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			shaderFile.open(path);
			std::stringstream shaderStream;
			shaderStream << shaderFile.rdbuf();
			shaderFile.close();
			code = shaderStream.str();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << "\n";
		}

		unsigned cs;
		cs = glCreateShader(GL_COMPUTE_SHADER);
		const char* cCode = code.c_str();
		glShaderSource(cs, 1, &cCode, NULL);
		glCompileShader(cs);

		GLint success;
		GLchar infoLog[1024];
		glGetShaderiv(cs, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(cs, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "cs" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			return;
		}

		id = glCreateProgram();
		glAttachShader(id, cs);
		glLinkProgram(id);
		glDeleteShader(cs);
		
	}
	void Use() const&;

	void SetTexture(const std::string&, const Texture& tex) &;
	void SetInt(const std::string&, int) const&;
	void SetFloat(const std::string&, float) const&;
	void SetVec4(const std::string&, const glm::vec4&) const&;
	void SetVec3(const std::string&, const glm::vec4&) const&;
	void SetVec2(const std::string&, const glm::vec4&) const&;

	void Dispath(int x, int y, int z) const&;
	void DispathIndirect() const&;
};


#endif // !COMPUTE_SHADER_H
