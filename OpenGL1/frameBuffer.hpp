#pragma once

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include"texture.hpp"

#define INVALID_FRAMEBUFFER_ID 0x777ffff

struct FrameBuffer
{
    unsigned frameBuffer;
    unsigned depthRBO;
    unsigned attachOffset;
    int w; int h;
    std::vector<Texture*> textures;
    FrameBuffer();
    FrameBuffer(unsigned w, unsigned h, bool addDepthRBO = false);
    ~FrameBuffer();
    void AttachTexture(Texture*,unsigned cnt=1);
    Texture* GetTexture(unsigned) const;
    FrameBuffer& operator=(const FrameBuffer&);
    void Construct(unsigned w, unsigned h, bool addDepthRBO = false);
    void Bind();

    //void AttachRenderBuffer(const RenderBuffer& rb);
};

using sp_FrameBuffer = std::shared_ptr<FrameBuffer>;

const FrameBuffer TargetOutputFrameBuffer = {0,0,false};

const std::shared_ptr<FrameBuffer> spTargetOutputFrameBuffer=std::make_shared<FrameBuffer>(0,0,false);

const FrameBuffer InvalidFrameBuffer;

const std::shared_ptr<FrameBuffer> spInvalidFrameBuffer = std::make_shared<FrameBuffer>();

#endif // !FRAME_BUFFER_H
