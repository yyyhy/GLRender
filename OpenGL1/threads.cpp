

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

void Task::SetState(TaskState s)
{
	state = s;
}

Task::~Task()
{
}

VirtualThread::VirtualThread() :doingTaskThread(nullptr),forceClose(false) {

}

VirtualThread::~VirtualThread() {

}

std::queue<std::shared_ptr<Task>>& VirtualThread::GetRestWaitingTasks(){
	return doingTasksQueue;
}

VirtualThreadsPool::~VirtualThreadsPool() {

}

BasicThread::BasicThread() :VirtualThread(), end(false)/*, checkThread(new std::thread(Check, &waitingTasks, &doingTasksQueue, &end, &this->forceClose)) */{

}

BasicThread::~BasicThread() {
	ForceClose();
}

unsigned BasicThread::ThreadPressure() {
	return /*waitingTasks.size() + */doingTasksQueue.size();
}

void BasicThread::Run() {
	doingTaskThread = new std::thread(DoTask, &doingTasksQueue, &end,&this->forceClose);
}

bool BasicThread::AddTask(std::shared_ptr<Task> task) {
	/*if (doingTasksQueue.size() >= MAX_TASK_SIZE||end)
		return false;*/
	doingTasksQueue.push(task);
	return true;
}

void BasicThread::Join() {
	end = true;
	/*if(checkThread->joinable())
		checkThread->join();*/
	if(doingTaskThread && doingTaskThread->joinable())
		doingTaskThread->join();
}

void BasicThread::ForceClose() {
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

void BasicThread::Wait()
{
	std::unique_lock<std::mutex> lock(m_conditional_mutex);
	m_conditional_lock.wait(lock);
}

std::thread::id BasicThread::GetThreadID()
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

unsigned BasicThreadsPool::ThreadPressure()
{
	return 0;
}

bool BasicThreadsPool::AddTask(std::shared_ptr<Task> task) {
	bool success=threadsPool[currThreadIndex]->AddTask(task);
	currThreadIndex++;
	currThreadIndex %= threadsPool.size();
	return success;
}

void BasicThreadsPool::Run(){
	for(auto var : threadsPool)
	{
		var->Run();
	}
}

void BasicThreadsPool::Join()
{
	for (auto var : threadsPool)
	{
		var->Join();
	}
}

void BasicThreadsPool::ForceClose()
{
	for (auto var : threadsPool)
	{
		var->ForceClose();
	}
}

bool BasicThreadsPool::AllTasksIsFinished()
{
	for (auto var : threadsPool)
	{
		if (var->ThreadPressure() != 0)
			return false;	
	}
	return true;
}

void BasicThreadsPool::Wait()
{
}

std::thread::id BasicThreadsPool::GetThreadID()
{
	return std::thread::id();
}
