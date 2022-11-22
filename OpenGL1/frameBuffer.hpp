#pragma once

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include"texture.hpp"

#define INVALID_FRAMEBUFFER_ID 0x777ffff

struct FrameBufferO
{
    unsigned frameBuffer;
    unsigned depthRBO;
    unsigned attachOffset;
    int w; int h;
    std::vector<Texture*> textures;
    FrameBufferO();
    FrameBufferO(unsigned w, unsigned h, bool addDepthRBO = false);
    ~FrameBufferO();
    void AttachTexture(Texture*,unsigned cnt=1);
    Texture* GetTexture(unsigned) const;
    FrameBufferO& operator=(const FrameBufferO&);
    void Construct(unsigned w, unsigned h, bool addDepthRBO = false);
    void Bind();

    //void AttachRenderBuffer(const RenderBuffer& rb);
};

using sp_FrameBufferO = std::shared_ptr<FrameBufferO>;

const FrameBufferO TargetOutputFrameBuffer = {0,0,false};

const std::shared_ptr<FrameBufferO> spTargetOutputFrameBuffer=std::make_shared<FrameBufferO>(0,0,false);

const FrameBufferO InvalidFrameBuffer;

const std::shared_ptr<FrameBufferO> spInvalidFrameBuffer = std::make_shared<FrameBufferO>();

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
