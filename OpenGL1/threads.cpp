

#include"threads.hpp"
#include"basicThread.hpp"
#include<iostream>



void Task::DebugTaskInformation()
{
}

TaskState Task::GetState() const
{
	return state;
}

void Task::WaitUntilTaskIsFinish()
{
	while (state != Finish);
}

void Task::SetState(TaskState s)
{
	state = s;
}

Task::~Task()
{
}

void Task::operator()()
{
	DoTask();
}

VirtualThread::VirtualThread() :doingTaskThread(nullptr),forceClose(false) {

}

VirtualThread::~VirtualThread() {

}

inline std::queue<std::shared_ptr<Task>>& VirtualThread::GetRestWaitingTasks(){
	return doingTasksQueue;
}

VirtualThreadsPool::~VirtualThreadsPool() {

}

BasicThread::BasicThread() :end(false)/*, checkThread(new std::thread(Check, &waitingTasks, &doingTasksQueue, &end, &this->forceClose)) */{

}

BasicThread::~BasicThread() {
	ForceClose();
}

inline unsigned BasicThread::ThreadPressure() {
	return /*waitingTasks.size() + */doingTasksQueue.size();
}

inline void BasicThread::Run() {
	doingTaskThread = new std::thread(DoTask, &doingTasksQueue, &end,&this->forceClose);
}

inline std::future<void> BasicThread::AddTask(std::shared_ptr<Task> task) {
	
	std::function<void()> func = std::bind((&Task::DoTask),  task.get());
	auto package = std::make_shared<std::packaged_task<void()>>(func);
	doingTasksQueue.push(task);
	return package->get_future();
}


inline void BasicThread::Join() {
	end = true;
	/*if(checkThread->joinable())
		checkThread->join();*/
	if(doingTaskThread && doingTaskThread->joinable())
		doingTaskThread->join();
}

inline void BasicThread::ForceClose() {
	forceClose = true;
	Join();
	if (doingTasksQueue.size() > 0) {
		std::cout << "BasicThreadID: " << GetThreadID()<<"\n";
		std::cout << doingTasksQueue.size() << " Tasks is still wating\n";
		while (doingTasksQueue.size())
		{
			auto task = doingTasksQueue.front();
			task->DebugTaskInformation();
			doingTasksQueue.pop();
		}
	}
		
	/*if(waitingTasks.size()>0)
		std::cout<< waitingTasks.size() << "Tasks is still doing\n";*/
}

inline void BasicThread::Wait()
{
	std::unique_lock<std::mutex> lock(m_conditional_mutex);
	m_conditional_lock.wait(lock);
}

inline std::thread::id BasicThread::GetThreadID()
{
	return doingTaskThread->get_id();
}

BasicThreadsPool::BasicThreadsPool(int cnt):currThreadIndex(0) {
	for (int i = 0; i < cnt; ++i)
		threadsPool.push_back(std::make_shared<BasicThread>());
}

BasicThreadsPool::~BasicThreadsPool()
{
	ForceClose();
}

inline unsigned BasicThreadsPool::ThreadPressure()
{
	return 0;
}

inline std::future<void> BasicThreadsPool::AddTask(std::shared_ptr<Task> task) {
	auto success=threadsPool[currThreadIndex]->AddTask(task);
	currThreadIndex++;
	currThreadIndex %= threadsPool.size();
	return success;
}



inline void BasicThreadsPool::Run(){
	for(auto var : threadsPool)
	{
		var->Run();
	}
}

inline void BasicThreadsPool::Join()
{
	for (auto var : threadsPool)
	{
		var->Join();
	}
}

inline void BasicThreadsPool::ForceClose()
{
	for (auto var : threadsPool)
	{
		var->ForceClose();
	}
}

inline bool BasicThreadsPool::AllTasksIsFinished()
{
	for (auto var : threadsPool)
	{
		if (var->ThreadPressure() != 0)
			return false;	
	}
	return true;
}

inline void BasicThreadsPool::Wait()
{
}

inline std::thread::id BasicThreadsPool::GetThreadID()
{
	return std::thread::id();
}

FunctionTask::FunctionTask(std::function<void()> f):func(std::move(f))
{
}

void FunctionTask::DoTask()
{
	func();
}

