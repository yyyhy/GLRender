

#ifndef BLIT_H
#define BLIT_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"



class Blit :public PostProcess {

private:
	int f = 1;
	ComputeShader cs;
public:
	Blit(unsigned w, unsigned h) 
		:PostProcess("shaders/bf.vs", "shaders/blit.fs", w, h),
		cs("shaders/cs/test.csr")
		{}

	void excute() override {
		auto s = GetShader();
		cs.SetTexture("sadimage", GetInTexBuffer());
		cs.SetBindingImage(0, GetInTexBuffer());
		cs.SetBindingImage(1, GetInTexBuffer());
		cs.SetFloat("f", 2);
		ComputeBuffer ssbo(sizeof(float) * 3, 8);
		std::vector<glm::vec3> v;
		for (int i = 0; i < 8; ++i)
			v.emplace_back(0.2, 0.2, 0.2);

		ssbo.SetData(&v, sizeof(float) * 3 * 8);
		cs.SetBuffer(2, ssbo);
		//cs.SetVec3("offset", glm::vec3(1, 0, 0));
		cs.Dispath(width, height, 1);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), s.get(),GetOutFrameBuffer());
	}
};

#endif // !BLIT_H
