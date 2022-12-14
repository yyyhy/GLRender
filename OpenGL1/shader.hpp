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
    

    Shader(const char* vertexPath, const char* fragmentPath, 
        const char* geometryPath = nullptr,std::vector<std::string>* preComplieCmd=NULL)
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
        if (preComplieCmd) {
            for (auto& i : *preComplieCmd) {
                vertexCode += "\n#define " + i + "\n";
            }
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

        use();
        setCubeMap("reflectCube[0].reflectCube", 0);
        setBool("reflectCube[0].exist", false);
        setCubeMap("reflectCube[1].reflectCube", 0);
        setBool("reflectCube[1].exist", false);
        setCubeMap("reflectCube[2].reflectCube", 0);
        setBool("reflectCube[2].exist", false);
        setCubeMap("reflectCube[3].reflectCube", 0);
        setBool("reflectCube[3].exist", false);
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

    void setTexture(const std::string& name, const std::string& path) {
        Texture2D tex2D(path);
        tex2D.name = name;
        if (tex2D.name != "albedoMap") {
            delete tex2D.albedo;
            tex2D.albedo = nullptr;
        }
        else {

        }
        textures[tex2D.name] = tex2D;
        stageChange = true;
    }

    void setTexture(const std::string& name, int id, bool is3D=false) {
        Texture t;
        t.name = name;
        t.id = id;
        if (is3D)
            t.type = GL_TEXTURE_3D;
        textures[name] = t;
        stageChange = true;
    }

    void setTexture(const std::string& name, Texture& t) {
        t.name = name;
        textures[name] = t;
    }

    void setTexture(const std::string& name, sp_texture t) {
        t->name = name;
        textures[name] = *t;
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
            if(i->second.type==GL_TEXTURE_2D)
                glBindTexture(GL_TEXTURE_2D, i->second.id);
            else {
                glBindTexture(GL_TEXTURE_3D, i->second.id);
            }
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