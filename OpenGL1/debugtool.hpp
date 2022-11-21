

#ifndef DEBUG_TOOL_H
#ifdef _DEBUG
#define DEBUG_TOOL_H
#include"frameBuffer.hpp"
#include"stb.hpp"

// static API

static std::vector<std::string> g_supportExtensions;
inline void GetSupportExtensions() {
	if (!g_supportExtensions.empty())
		return;
	GLint n, i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (i = 0; i < n; i++) {
		std::string extension = (char*)glGetStringi(GL_EXTENSIONS, i);
		g_supportExtensions.push_back(extension);
	}
}

// public API
inline bool CheckExtension(const std::string& extensionName) {
	GetSupportExtensions();

	for (int i = 0; i < g_supportExtensions.size(); i++) {
		if (g_supportExtensions[i] == extensionName)
			return true;
	}
	return false;
}

inline void SaveFrameBuffer(const FrameBufferO& fbo) {
	unsigned char* imageData = new unsigned char[fbo.w * fbo.h * 3] { 255 };
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer);
	glReadPixels(0, 0, fbo.w, fbo.h, GL_RGB, GL_UNSIGNED_BYTE, (unsigned char*)imageData);
	std::string file = "baking/capture.png";
	stbi_write_png(file.c_str(), fbo.w, fbo.h, 3, imageData, 0);
	stbi_flip_vertically_on_write(1);
	delete[] imageData;
}


#endif // DEBUG



#endif