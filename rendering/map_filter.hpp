

#ifndef MAP_FILTER_H
#define MAP_FILTER_H

#include"frame_buffer.hpp"
#include"opengl.hpp"
#include"shader.hpp"
#include"glm.hpp"
#include"reflect_probe.hpp"
#include"commons.hpp"

using mipmap = unsigned;
static unsigned int cubeVAO = 0;
static unsigned int cubeVBO = 0;
static void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

static unsigned planeVAO=0;
static unsigned planeVBO=0;
static unsigned planeFrameBuffer = 0;
static void renderPlane() {
	if (planeVAO == 0) {
		glGenVertexArrays(1, &planeVAO);
		glBindVertexArray(planeVAO);
		glGenBuffers(1, &planeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glBindVertexArray(0);
	}
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

mipmap GenCubeMipMap(Shader * prefilterShader,const FrameBuffer Buffer, unsigned level = 4,bool over=false) {

	unsigned prefilterMap;
	if (over) {
		glGenTextures(1, &prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, Buffer.w, Buffer.h, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
		prefilterMap = Buffer.texBuffer;
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	prefilterShader->use();
	prefilterShader->setMat4("projection", glm::perspective(glm::radians(90.f), 1.f, 0.1f, 100.f));
	prefilterShader->setCubeMap("environmentMap", Buffer.texBuffer);
	prefilterShader->initTexture();

	glBindFramebuffer(GL_FRAMEBUFFER, Buffer.frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, Buffer.rbo);
	for (int mip = 1; mip < level; mip++) {
		unsigned int mipWidth = Buffer.w * std::pow(0.5, mip);
		unsigned int mipHeight = Buffer.h * std::pow(0.5, mip);
		glViewport(0, 0, mipWidth, mipHeight);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mipWidth, mipHeight);
		float roughness = (float)mip / (float)(level - 1);
		prefilterShader->setFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader->setMat4("view", glm::lookAt({ 0,0,0 }, captureViews[i * 2], captureViews[i * 2 + 1]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
			glClear(GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return prefilterMap;
}

mipmap GenTexMipMap(Shader* prefilterShader, const  FrameBuffer Buffer, unsigned level = 4,unsigned w=512,unsigned h=512) {
	unsigned prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_2D, prefilterMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	prefilterShader->use();
	prefilterShader->setTexture("prefilterInMap", Buffer.texBuffer);
	prefilterShader->initTexture();

	glBindFramebuffer(GL_FRAMEBUFFER, Buffer.frameBuffer);
	for (int mip = 0; mip < level; mip++) {
		unsigned int mipWidth = w * std::pow(0.5, mip);
		unsigned int mipHeight = h * std::pow(0.5, mip);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(level - 1);
		prefilterShader->setFloat("roughness", roughness);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prefilterMap, mip);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderPlane();

		prefilterShader->setTexture("prefilterInMap", prefilterMap);
		prefilterShader->initTexture();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return prefilterMap;
}

void BlitMap(unsigned in,const FrameBuffer& out,Shader *s) {

	s->use();
	s->setTexture("tex", in);
	s->initTexture();
	
	glBindFramebuffer(GL_FRAMEBUFFER, out.frameBuffer); 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out.texBuffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderPlane();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif // !CUBEMAP_FILTER_H
