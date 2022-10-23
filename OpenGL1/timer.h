
#ifndef TIMER_H
#define TIMER_H
class Timer {
private:
	volatile float lastFrameTime;
	volatile float currFrameTime;
public:
	Timer():lastFrameTime(0),currFrameTime(0),deltaTime(0){}

	void updateTime(float t) {
		lastFrameTime = currFrameTime;
		currFrameTime = t;
		deltaTime = currFrameTime - lastFrameTime;
	}

	volatile float deltaTime;
};

extern Timer globalTimer;

#endif // !TIMER_H


