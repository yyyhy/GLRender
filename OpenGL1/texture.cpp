#include"texture.hpp"
#include"frameBuffer.hpp"
#include"stb_image.h"
#include<iostream>

Texture2D::Texture2D(const std::string& path) :Texture(GL_TEXTURE_2D)
{
	int w, h,channel;
	albedo = stbi_load(path.c_str(), &w, &h, &channel, 0);
    unsigned TEX;
    glGenTextures(1, &TEX);
    glBindTexture(GL_TEXTURE_2D, TEX);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    auto type = channelMap[channel];
    glTexImage2D(GL_TEXTURE_2D, 0, type, w, h, 0, type, GL_UNSIGNED_BYTE, albedo);

    glGenerateMipmap(GL_TEXTURE_2D);
    this->id = TEX;
    this->w = w;
    this->h = h;
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::Texture2D(unsigned w, unsigned h, unsigned iFormat, unsigned format, 
    unsigned MinFilter,unsigned MagFilter,unsigned WrapS,unsigned WrapT,
    bool GenMip) :Texture(GL_TEXTURE_2D)
{
    this->w = w;
    this->h = h;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, iFormat, w, h, 0, format, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapS);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapT);
    if(GenMip)
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D()
{
   
}

void Texture2D::Release()
{
    if (id != INVALID_TEXTURE_ID && id != 0)
        glDeleteBuffers(1, &id);
}

void Texture2D::Construct(unsigned w, unsigned h, unsigned iFormat, unsigned format,
    unsigned MinFilter, unsigned MagFilter, unsigned WrapS, unsigned WrapT,
    bool GenMip){
    if (id != INVALID_TEXTURE_ID) {
        std::cout << "Can't construct a valid texture\n";
        return;
    }

    this->w = w;
    this->h = h;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, iFormat, w, h, 0, format, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapT);
    if (GenMip)
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture3D::Texture3D(float* data, int w, int h, int d, unsigned iFormat,unsigned format):Texture(GL_TEXTURE_3D),d(d)
{
    this->w = w;
    this->h = h;
    this->d = d;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, iFormat, w, h, d, 0, format, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture3D::Texture3D(int w, int h, int d, unsigned channel):Texture(GL_TEXTURE_3D)
{
    this->w = w;
    this->h = h;
    this->d = d;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, channel, w, h, d, 0, channel, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture::~Texture()
{
    
}

void Texture::Release()
{
    
}

FrameBuffer::FrameBuffer():w(0),h(0),attachOffset(0),depthRBO(0),frameBuffer(INVALID_FRAMEBUFFER_ID)
{
    
}

FrameBuffer::FrameBuffer(unsigned w, unsigned h, bool addDepthRBO):attachOffset(0),depthRBO(0),w(w),h(h)
{
    if (w > 0 && h > 0) {
        glGenFramebuffers(1, &frameBuffer);
        if (addDepthRBO) {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glGenRenderbuffers(1, &depthRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
        frameBuffer = 0;
}

void FrameBuffer::Construct(unsigned w, unsigned h, bool addDepthRBO ) {
    if (frameBuffer != INVALID_FRAMEBUFFER_ID) {
        std::cerr << "Can't construct a valid framebuffer\n";
        return;
    }
    if (w > 0 && h > 0) {
        this->w = w;
        this->h = h;
        glGenFramebuffers(1, &frameBuffer);
        if (addDepthRBO) {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glGenRenderbuffers(1, &depthRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
        frameBuffer = 0;
}

FrameBuffer::~FrameBuffer()
{
    if (frameBuffer != 0&&frameBuffer!=INVALID_FRAMEBUFFER_ID) {
        glDeleteFramebuffers(1, &frameBuffer);
    }
    if (depthRBO != 0) {
        glDeleteRenderbuffers(1, &depthRBO);
    }
}

FrameBuffer& FrameBuffer::operator=(const FrameBuffer& fbo)
{
    
    frameBuffer = fbo.frameBuffer;
    depthRBO = fbo.depthRBO;
    attachOffset = fbo.attachOffset;
    w = fbo.w;
    h = fbo.h;
    textures = fbo.textures;
    return *this;
}

void FrameBuffer::AttachTexture(Texture* t,unsigned cnt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    GLuint* arrays = new GLuint[cnt];
    for (int i = 0; i < cnt; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (attachOffset+i), t[i].type, t[i].id, 0);
        arrays[i] = GL_COLOR_ATTACHMENT0 + (attachOffset + i);
        textures.push_back(&t[i]);
    }
    
    attachOffset += cnt;
    if(cnt>1)
        glDrawBuffers(attachOffset, arrays);
}

Texture* FrameBuffer::GetTexture(unsigned index) const
{
    if(index>=textures.size())
        return nullptr;

    return textures[index];
}

void FrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, w, h);
}

TextureCube::TextureCube(int w, int h,unsigned iFormat, unsigned format,
    unsigned MinFilter, unsigned MagFilter, unsigned WrapS, unsigned WrapT, unsigned WrapR,
    bool GenMip):Texture(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
{
    this->w = w;
    this->h = h;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, iFormat, w, h, 0, format, GL_FLOAT, NULL
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);

    if(GenMip)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void TextureCube::Release()
{
    if (id != INVALID_TEXTURE_ID && id != 0)
        glDeleteBuffers(1, &id);
}

TextureCube::~TextureCube()
{
    
}
