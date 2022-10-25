#include"texture.hpp"
#include"frame_buffer.hpp"
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

Texture2D::Texture2D(unsigned w, unsigned h, unsigned iFormat, unsigned format) :Texture(GL_TEXTURE_2D)
{
    this->w = w;
    this->h = h;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, iFormat, w, h, 0, format, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture3D::Texture3D(float* data, int w, int h, int d, unsigned channel):Texture(GL_TEXTURE_3D),d(d)
{
    this->w = w;
    this->h = h;
    this->d = d;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, channel, w, h, d, 0, channel, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture3D::Texture3D(int w, int h, int d, unsigned channel):Texture(GL_TEXTURE_3D)
{
    this->w = w;
    this->h = h;
    this->d = d;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, channel, w, h, d, 0, channel, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_3D, 0);
}

Texture::~Texture()
{

}

void Texture::Release()
{
    if (id != 0)
        glDeleteBuffers(1, &id);
}

FrameBufferO::FrameBufferO(unsigned w, unsigned h, bool addDepthRBO):attachOffset(0),depthRBO(0),w(w),h(h)
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
        else
            std::cout << frameBuffer << " is created"<<std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
        frameBuffer = 0;
}

FrameBufferO::~FrameBufferO()
{
    if (frameBuffer != 0) {
        glDeleteFramebuffers(1, &frameBuffer);
    }
    if (depthRBO != 0) {
        glDeleteRenderbuffers(1, &depthRBO);
    }
}

void FrameBufferO::AttachTexture(Texture* t,unsigned cnt)
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

void FrameBufferO::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, w, h);
}

TextureCube::TextureCube(int w, int h):Texture(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
{
    this->w = w;
    this->h = h;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}
