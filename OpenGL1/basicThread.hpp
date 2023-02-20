

#ifndef BASIC_THREAD_H
#define BASIC_THREAD_H
#include"threads.hpp"
#include<memory>

class BasicThread : public VirtualThread {

private:
	bool end;
	const unsigned MAX_TASK_SIZE = 64;
	std::mutex m_conditional_mutex;
	std::condition_variable m_conditional_lock;
public:

	BasicThread();

	virtual ~BasicThread() override;

	virtual unsigned ThreadPressure() override;

	virtual std::future<void> AddTask(std::shared_ptr<Task> task) override;

	virtual void Run() override;

	virtual void Join() override;

	virtual void ForceClose() override;

	virtual void Wait() override;

	virtual std::thread::id GetThreadID() override;

};

inline void DoTask(std::queue<std::shared_ptr<Task>>* doingTasks, bool* end, bool* forceClose) {
	while ((!((*end) && doingTasks->size() == 0))&&!(*forceClose))
	{
		
		if (doingTasks->size()) {
			doingTasks->front()->DoTask();
			doingTasks->front()->SetState(Finish);
			doingTasks->pop();
		}

	}

}

inline void Check(std::queue<std::shared_ptr<Task>>* doingTasks, std::list<std::shared_ptr<Task>>* waitingTasks, bool* end, bool* forceClose) {

	while ((!((*end) && waitingTasks->size() == 0)))
	{
		
		for (auto task = waitingTasks->begin(); task != waitingTasks->end();) {
			if (task->get()->GetState() == Begin) {
				task->get()->SetState(Doing);
				doingTasks->push(*task);
				++task;
			}
			else if (task->get()->GetState() == Finish) {
				task = waitingTasks->erase(task);
			}
			else
				++task;
		}
	}

}

class BasicThreadsPool : public VirtualThreadsPool {
private:
	unsigned currThreadIndex;
	
	
public:
	BasicThreadsPool(int threadCnt);

	virtual ~BasicThreadsPool() override;

	virtual unsigned ThreadPressure() override;

	virtual std::future<void> AddTask(std::shared_ptr<Task>) override;

	virtual void Run() override;

	virtual void Join() override;

	virtual void ForceClose() override;

	virtual bool AllTasksIsFinished() override;

	virtual void Wait() override;

	virtual std::thread::id GetThreadID() override;

};

#endif // !BASIC_THREAD_H


