
#ifndef LIGHT_FLICKER_H
#define LIGHT_FLICKER_H

#include"component.hpp"
#include"light.hpp"
#include"object.hpp"
#include"timer.h"

class LightFlicker :public Component {
private:
	Light* l;
	float timer;
	float dir = 1;
public:
	LightFlicker():Component(),l(nullptr),timer(2){}

	void Update() override {

		if (!l) {
			l= object->GetComponent<Light>();
		}

		else {
			timer += dir*globalTimer.deltaTime*5;
			if (timer >= 50 || timer <= 0.2f)
				dir = -dir;

			l->SetIntensity(timer);
		}
	}

	
};

#endif // !LIGHT_FLICKER_H
