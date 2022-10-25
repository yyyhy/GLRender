#pragma once

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include"texture.hpp"

struct FrameBufferO
{
    unsigned frameBuffer;
    unsigned depthRBO;
    unsigned attachOffset;
    int w; int h;
    std::vector<Texture*> textures;
    FrameBufferO(unsigned w, unsigned h, bool addDepthRBO = false);
    ~FrameBufferO();
    void AttachTexture(Texture* t,unsigned cnt=1);
    void Bind();

    //void AttachRenderBuffer(const RenderBuffer& rb);
};

const FrameBufferO emptyBufferO = {0,0,false};


struct FrameBuffer
{
    unsigned frameBuffer;
    unsigned texBuffer;
    unsigned rbo;
    unsigned w;
    unsigned h;
};

const FrameBuffer emptyBuffer = { 0,0,0,0,0 };
#endif // !FRAME_BUFFER_H
