#pragma once

#ifndef DRAW_ADAPTER_H
#define DRAW_ADAPTER_H

#include<memory>
#include<vector>
#include"shader.hpp"
class Draw {
public:
	virtual void draw(unsigned VAO,unsigned size) = 0;
};

class NormalDraw: public Draw {
private:
	std::shared_ptr<Shader> shader;
public:
	NormalDraw(std::shared_ptr<Shader> shader):shader(shader){}

	void draw(unsigned VAO, unsigned size) override{
		if (shader != NULL) {
			shader->InitTexture();
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
		}
	}
};

class InstanceDraw : public Draw {
private:
private:
	std::shared_ptr<Shader> shader;
	unsigned cnt;
public:
	InstanceDraw(std::shared_ptr<Shader> shader,unsigned cnt) :shader(shader),cnt(cnt) {}

	void draw(unsigned VAO, unsigned size) override {
		if (shader != NULL) {
			shader->InitTexture();
			glBindVertexArray(VAO);
			glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0,cnt);
		}
	}

};

#endif // !DRAW_ADAPTER_H
