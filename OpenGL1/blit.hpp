

#ifndef BLIT_H
#define BLIT_H
#include"cPostProcess.hpp"
#include"computeShader.hpp"


extern "C" {
	struct SSBO
	{
		glm::vec3 col; char tmp0[4];
		glm::vec3 col1; char tmp1[4];
		
	};
}


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
		cs.SetTexture("sadimage", 11);
		cs.SetTexture("RSM", 15);
		cs.SetBindingImage(0, GetInTexBuffer());
		/*cs.SetFloat("f", 2);
		ComputeBuffer ssbo(sizeof(SSBO), 4);
		std::vector<SSBO> v;
		for (int i = 0; i < 4; ++i) {
			SSBO csr;
			csr.col = glm::vec3(1, 0, 1);
			v.push_back(std::move(csr));
		}
			

		ssbo.SetData(&v[0]);
		cs.SetBuffer(2, ssbo);
		cs.SetVec3("offset", glm::vec3(1, 0, 0));
		
		SSBO* data;
		data=(SSBO*) ssbo.ReadData();
		for (int i = 0; i < 4; ++i)
			std::cout << data[i].col.x<<" "<<data[i].col.y<<" "<<data[i].col.z<<"  " ;
		std::cout << "\n";*/
		//cs.Dispath(width, height, 1);
		BlitMap(GetInTexBuffer(), GetOutTexBuffer(), s.get(),GetOutFrameBuffer());
	}
};

#endif // !BLIT_H
