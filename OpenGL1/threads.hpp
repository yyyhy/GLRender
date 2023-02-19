#pragma once


#ifndef TASH_H
#define TASK_H
#include<thread>
#include<memory>
#include<vector>
#include<queue>
#include<list>
#include<mutex>
#include<iostream>

enum TaskState
{
	Finish,Doing,Begin
};

class Task {
protected:
	TaskState state;
public:
	
	virtual ~Task();
	virtual void DoTask() = 0;
	virtual void DebugTaskInformation();
	TaskState GetState() const;
	void SetState(TaskState);
};

class TaskGroup : Task {
private:
	std::vector<Task> tasks;

	
};

class VirtualThread {
protected:
	bool forceClose;
	std::thread* doingTaskThread;
	std::queue<std::shared_ptr<Task>> doingTasksQueue;

public:
	VirtualThread();
	virtual ~VirtualThread();
	virtual unsigned ThreadPressure() = 0;
	virtual void Run() = 0;
	virtual bool AddTask(std::shared_ptr<Task>) = 0;
	virtual void Join() = 0;
	virtual void ForceClose() = 0;
	virtual std::thread::id GetThreadID() = 0;
	virtual void Wait() = 0;
	std::queue<std::shared_ptr<Task>>& GetRestWaitingTasks();

};

class VirtualThreadsPool : public VirtualThread{
protected:
	std::vector<std::shared_ptr<VirtualThread>> threadsPool;

public:

	virtual ~VirtualThreadsPool();

	virtual bool AllTasksIsFinished() = 0;

};

class TestTask : public Task {
public:
	int n;
	TestTask(int n) :n(n) { this->state = Begin; }

	void DoTask() override {
		for (float i = 0; i < n; i += 100.f) {
			std::cout << n << " is finshied\n";
		}
		
	}

	void DebugTaskInformation() override{
		std::cout << "TestTask: " << "n: " << n << "\n";
	}

};

#endif // !TASH_H
