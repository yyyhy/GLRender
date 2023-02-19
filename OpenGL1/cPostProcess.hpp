

#ifndef C_POST_PROCESS_H
#define C_POST_PROCESS_H

#include"shader.hpp"
#include"settings.hpp"
#include"frameBuffer.hpp"
#include"filter.hpp"


class PostProcess {
private:
	std::shared_ptr<Shader> shader;
	FrameBuffer* FrameBuffers;
	Texture InTexture;
	Texture* OutTextures;
	unsigned size;

	bool needClearBuffers = true;
protected:
	unsigned width, height;
public:
	PostProcess(const char* vertexPath, const char* fragmentPath, unsigned w, unsigned h, unsigned bufferSize = 1, std::vector<std::string>* preComplieCmd = NULL)
		:enable(true),
		size(bufferSize),
		width(w),
		height(h) {
		if (vertexPath != nullptr && fragmentPath != nullptr)
			shader = std::make_shared<Shader>(vertexPath, fragmentPath, nullptr, preComplieCmd);

	}

	PostProcess(const std::shared_ptr<Shader> s) noexcept {
		shader = s;
	}

	PostProcess() = default;

	virtual	~PostProcess() {
		if (needClearBuffers) {
			if (FrameBuffers != nullptr && size > 0)
				delete[] FrameBuffers;
			if (OutTextures != nullptr && size > 0) {
				for (int i = 0; i < size; ++i) {
					OutTextures[i].Release();
				}
				delete[] OutTextures;
			}

		}
	}

	virtual void GenFrameBuffer() {
		FrameBuffers = new FrameBuffer[size]();
		OutTextures = new Texture2D[size];
		for (int i = 0; i < size; ++i) {
			FrameBuffers[i].Construct(width, height, false);
			OutTextures[i] = Texture2D(width, height, GL_RGBA32F, GL_RGBA);
			FrameBuffers[i].AttachTexture(&OutTextures[i]);
		}

	}

	virtual void SendBufferToNext(PostProcess* p) {
		if (p)
			p->SetInTexBuffer(OutTextures[0]);
	}

	void SetInTexBuffer(const Texture& buffer) {
		InTexture = buffer;
		if (shader) {
			shader->use();
			shader->SetTexture("tex", InTexture);
		}
	}

	void SetOutFrameBuffer(FrameBuffer* outBuffer) {
		FrameBuffers = outBuffer;
		needClearBuffers = false;
	}

	void SetOutTexBuffer(Texture* outTex) {
		OutTextures = outTex;
		needClearBuffers = false;
	}

	const Texture& GetOutTexBuffer(unsigned index = 0) const {
		if (OutTextures != nullptr && index < size)
			return OutTextures[index];
		return TargetOuputTexture;
	}

	const Texture& GetInTexBuffer() const { return InTexture; }

	const FrameBuffer& GetOutFrameBuffer(unsigned index = 0) const {
		if (FrameBuffers != nullptr && index < size)
			return FrameBuffers[index];
		return TargetOutputFrameBuffer;
	}

	virtual void excute() = 0;

	std::shared_ptr<Shader> GetShader() const { return shader; }
	bool enable;
};

class SSAO;
class SSDO;
class SSGI;
class BidirectionHFilter;
class BidirectionVFilter;
class Blit;
#endif // !POST_PROCESS_H
