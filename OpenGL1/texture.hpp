
#ifndef TEXTURE_H
#define TEXTURE_H
#include<string>
#include"opengl.hpp"
#include"bound.hpp"
#include<vector>

const unsigned channelMap[] = { 0, GL_RED,0,GL_RGB,GL_RGBA };

struct Texture {
    unsigned id;
    unsigned w;
    unsigned h;
    const unsigned char* albedo;
    unsigned type;
    std::string name;
    Texture(unsigned t = GL_TEXTURE_2D):type(t){}
    virtual ~Texture();
    virtual void Release();
    void swap(Texture&& t) {
        id = t.id;
        albedo = t.albedo;
        name = t.name;

        t.id = 0;
    }
};

struct Texture2D :public Texture {
    Texture2D():Texture(){}
    Texture2D(const std::string& path);
    Texture2D(unsigned w, unsigned h, unsigned iFormat,unsigned format);
};

struct Texture3D: public Texture{
    unsigned d;
    Bounds3 box;
    Texture3D(float* data, int w, int h, int d,unsigned channel);
    Texture3D(int w, int h, int d, unsigned channel);
};

struct TextureCube:public Texture
{
    TextureCube(int w, int h);
};

#endif // !TEXTURE_H
