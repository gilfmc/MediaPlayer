#include "TaskManager.h"

TaskManager::TaskManager()
{

}
void TaskManager::run(Task * task) {
	QMutexLocker lock(&mutex);
	tasks.append(task);
	task->run(this);
}

bool TaskManager::hasRunningTasks() {
	QMutexLocker lock(&mutex);
	bool running = false;
	for(Task * task : tasks) {
		if(task->isRunning()) {
			running = true;
			break;
		}
	}
	
	return running;
}

bool TaskManager::hasTaskWithName(const QString &name) {
	QMutexLocker lock(&mutex);
	for(Task * task : tasks) {
		if(task->name() == name) return true;
	}
	return false;
}

void TaskManager::onFinishedRunning(Task * task) {
	QMutexLocker lock(&mutex);
	// TODO this method is probably executed on an non-existing instance
	// when the app is closing and there are tasks still running
	int index = tasks.indexOf(task);
	if(index != -1) tasks.removeAt(index);
	if(!((QRunnable*)task)->autoDelete()) delete task;
}

TaskManager::~TaskManager() {
	for(Task * task : tasks) task->cancel();
}
