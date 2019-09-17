#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{
	while (true)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks

		std::unique_lock<std::mutex> lock(mtx);

		if (scheduledTasks.empty()) {
			e.wait(lock);
		}
		else {
			Task* to_execute = scheduledTasks.front();
			scheduledTasks.pop();

			to_execute->execute();

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

	for (int i = 0; i < MAX_THREADS; i++)
		threads[i].join();

	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread

	scheduledTasks.push(task);
	e.notify_one();
}
