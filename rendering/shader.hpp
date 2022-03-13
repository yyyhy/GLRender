#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include"glm.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include<map>
#include"texture.hpp"

enum RenderType {
    Opaque=0,Transparent=1,
};

enum CullType {
    FRONT=GL_FRONT,BACK=GL_BACK,BOTH=GL_FRONT_AND_BACK,
};

enum ShadowType
{
    RSM=0,PCF=1,PCSS=2
};

enum RenderPass {
    Forward=0,Deffered=1
};

const unsigned channelMap[] = {0, GL_RED,0,GL_RGB,GL_RGBA };

class Shader{
private:
    unsigned int ID;
    RenderType renderType;
    CullType cullType;
    ShadowType shadowType;
    mutable bool stageChange = true;
    std::map<std::string, Texture> textures;
    std::map<std::string, int> cubeMaps;
public:
    

    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        renderType = RenderType::Opaque;
        cullType = CullType::BACK;
        shadowType = ShadowType::RSM;
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            // if geometry shader path is present, also load a geometry shader
            if (geometryPath != nullptr)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout <<e.what()<< "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        //auto v=glCreateShader(GL_SHADER)
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr)
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);

    }
    
    ~Shader() {
#ifdef _DEBUG
        std::cout << ID << " shader lose\n";
#endif // DEBUG
        
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    {
        glUseProgram(ID);
    }
    
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }

    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }

    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setTexture(const std::string& name, const unsigned char* data,int w,int h,int channel) {

        unsigned TEX;
        glGenTextures(1, &TEX);
        glBindTexture(GL_TEXTURE_2D, TEX);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        auto type = channelMap[channel];
        glTexImage2D(GL_TEXTURE_2D, 0, type, w, h, 0, type, GL_UNSIGNED_BYTE, data);
        
        glGenerateMipmap(GL_TEXTURE_2D);
        Texture t;
        t.name = name;
        t.id = TEX;
        t.w = w;
        t.h = h;
        if (t.name != "albedoMap") {
            delete data;
            data = nullptr;
        }
        else {
            t.albedo = data;
        }
        textures[t.name] = t;
        stageChange = true;
    }

    void setTexture(const std::string& name, int id) {
        Texture t;
        t.name = name;
        t.id = id;
        textures[name] = t;
        stageChange = true;
    }

    void setCubeMap(const std::string& name, int id) {
        cubeMaps[name] = id;
        stageChange = true;
    }

    void SetRenderType(RenderType t) {
        renderType = t;
        setInt("shadowType", renderType);
    }

    void SetShadowType(ShadowType t) {
        shadowType = t;
        setInt("shadowType", shadowType);
    }

    RenderType GetRenderType() const { return renderType; }

    ShadowType GetShadowType() const { return shadowType; }

    void initTexture() const {
        /*if (!stageChange) {
            return;
        }*/
        int index = 0;
        for (auto i = textures.begin(); i != textures.end();i++,index++) {
            setInt(i->first, index);
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D, i->second.id);
            //std::cout << i->first << " " << i->second.id << "\n";
        }

        for (auto i = cubeMaps.begin(); i != cubeMaps.end(); i++, index++) {
            setInt(i->first, index);
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_CUBE_MAP, i->second);
        }
        stageChange = false;
    }

    Texture GetTexture(const std::string& name) {
        auto t = textures[name];
        return t;
    }

    void debug() {
        for (auto& i : textures) {
            std::cout << i.first << " " << i.second.id << "\n";
        }
    }
private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

using sp_shader = std::shared_ptr<Shader>;
#define CreatShader(v,f)=std::make_shared<Shader>(v,f)

#endif