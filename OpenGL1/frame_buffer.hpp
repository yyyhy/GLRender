#pragma once

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

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
