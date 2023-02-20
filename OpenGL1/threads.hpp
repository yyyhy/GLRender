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
#include <functional>
#include <future>

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

	void operator()();
};

class FunctionTask: public Task{
private:
	std::function<void()> func;

public:

	FunctionTask(std::function<void()>);

	virtual void DoTask() override;
};

class TaskGroup : Task {
private:
	std::vector<Task> tasks;

	
};

enum ThreadState
{
	ReadyToRun,Joined,Runing
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
	virtual std::future<void> AddTask(std::shared_ptr<Task>) = 0;
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

template<class F, class ...Args>
inline auto AddTask(VirtualThread* thread,F&& f, Args&& ...args)->std::future<decltype(f(args...))>
{
	std::function<decltype(f(args...))() > func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
	auto package = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

	std::function<void()> wrapperFunc = [package]() {
		(*package)();
	};

	thread->AddTask(std::make_shared<FunctionTask>(wrapperFunc));

	return package->get_future();

}

class TestTask : public Task {
public:
	int n;
	int ans;
	TestTask(int n) :n(n),ans(0) { this->state = Begin; }

	void DoTask() override {
		for (float i = 0; i < n; i += 100.f) {
			std::cout << n << " is finshied\n";
		}
		ans = n + 2;
	}

	void DebugTaskInformation() override{
		std::cout << "TestTask: " << "n: " << n << "\n";
	}

};

inline void TestFunc(int n) {
	for (float i = 0; i < n; i += 100.f) {
		std::cout << n << " is finshied\n";
	}
}
#endif // !TASH_H
