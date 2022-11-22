
#ifndef TEXTURE_H
#define TEXTURE_H
#include<string>
#include"opengl.hpp"
#include"bound.hpp"
#include<vector>
#include<memory>

const unsigned channelMap[] = { 0, GL_RED,0,GL_RGB,GL_RGBA };
#define INVALID_TEXTURE_ID 0x777ffff

struct Texture {
    unsigned id;
    unsigned w;
    unsigned h;
    const unsigned char* albedo;
    unsigned type;
    std::string name;
    Texture(unsigned t = GL_TEXTURE_2D,const std::string& name="") :type(t), id(0),name(name) {}
    virtual ~Texture();
    virtual void Release();
    void swap(Texture&& t) {
        id = t.id;
        albedo = t.albedo;
        name = t.name;

        t.id = 0;
    }
};

using sp_texture = std::shared_ptr<Texture>;

const Texture TargetOuputTexture;

const std::shared_ptr<Texture> spTargetOuputTexture = std::make_shared<Texture>();

struct Texture2D :public Texture {
    Texture2D() :Texture() { id = INVALID_TEXTURE_ID; }
    Texture2D(const std::string& path);
    Texture2D(unsigned w, unsigned h, unsigned iFormat,unsigned format, 
        unsigned MinFilter = GL_LINEAR, unsigned MagFilter = GL_LINEAR,
        unsigned WrapS=GL_REPEAT,unsigned WrapT=GL_REPEAT,
        bool GenMip=true);

    ~Texture2D();

    void Release() override;

    void Construct(unsigned w, unsigned h, unsigned iFormat, unsigned format,
        unsigned MinFilter = GL_LINEAR, unsigned MagFilter = GL_LINEAR,
        unsigned WrapS = GL_REPEAT, unsigned WrapT = GL_REPEAT,
        bool GenMip = true);
};

struct Texture3D: public Texture{
    unsigned d;
    Bounds3 box;
    Texture3D(float* data, int w, int h, int d,unsigned channel);
    Texture3D(int w, int h, int d, unsigned channel);
};

struct TextureCube:public Texture
{
    TextureCube(int w, int h,unsigned iFormat, unsigned format,
        unsigned MinFilter = GL_LINEAR, unsigned MagFilter = GL_LINEAR,
        unsigned WrapS= GL_CLAMP_TO_EDGE,unsigned WrapT= GL_CLAMP_TO_EDGE
        ,unsigned WrapR= GL_CLAMP_TO_EDGE,bool GenMip = true);

    void Release() override;

    ~TextureCube();
};

#endif // !TEXTURE_H
