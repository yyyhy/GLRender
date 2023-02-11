#pragma once


#ifndef TASH_H
#define TASK_H
#include<thread>

class Task {

public:
	virtual bool IsFinished() abstract;
	virtual unsigned TaskPressure() abstract;

	virtual void DoTask() abstract;

};
#endif // !TASH_H
