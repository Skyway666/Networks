#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{
	Task* to_execute = nullptr;
	while (true)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks
		{
			std::unique_lock<std::mutex> lock(mtx);

			while (scheduledTasks.empty() & !exitFlag) {
				e.wait(lock);			
			}

			if (exitFlag)
				break;
			else {
				to_execute = scheduledTasks.front();
				scheduledTasks.pop();
			}
		}

		to_execute->execute();
		{
			std::unique_lock<std::mutex> lock(mtx);
			finishedTasks.push(to_execute);
		}
	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())
	
	for (int i = 0; i < MAX_THREADS; i++)
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this);

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)

	std::unique_lock<std::mutex> lock(mtx);
	while(!finishedTasks.empty()) {
		Task* finished_t = finishedTasks.front();
		finished_t->owner->onTaskFinished(finished_t);
		finishedTasks.pop();
	}

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them

	{
		std::unique_lock<std::mutex> lock(mtx);
		exitFlag = true;
		e.notify_all();
	}

	for (int i = 0; i < MAX_THREADS; i++) 
		threads[i].join();
	

	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	{
		std::unique_lock<std::mutex> lock(mtx);
		scheduledTasks.push(task);
		e.notify_one();
	}
}
