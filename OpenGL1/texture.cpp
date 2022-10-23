#include"texture.hpp"
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
}

Texture2D::Texture2D(unsigned w, unsigned h,unsigned channel) :Texture(GL_TEXTURE_2D)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, channel, w, h, 0, channel, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture3D::Texture3D(float* data, int w, int h, int d, unsigned channel):Texture(GL_TEXTURE_3D),d(d)
{
    this->w = w;
    this->h = h;
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
