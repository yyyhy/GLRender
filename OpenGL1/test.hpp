

#include"object.hpp"
#include"timer.h"



class Test :public Component {
private:
	Transform* t;
	int dir = 1;
	int timer=0;
public:
	Test() :Component(),t(nullptr) { name = "test"; }

	void Start() override {
		
	}

	void Update() override {
		if (!t) {
			t = object->GetComponent<Transform>();
			if (!t) {
				std::cout << "null";
				return;
			}
		}
		timer++;
		if (timer > 500) {
			timer = 0;
			dir = -dir;
		}
		t->Translate(globalTimer.deltaTime * 0.5*dir,0, 0);
	}
};