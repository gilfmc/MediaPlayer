#include "Task.h"

void Task::run(TaskOwner * owner) {
	this->owner = owner;
	QThreadPool::globalInstance()->start(this);
}

void Task::run() {
	mutexRunning.lock();
	running = true;
	mutexRunning.unlock();
	
	_cancel = false;
	onRun();
	
	mutexRunning.lock();
	running = true;
	mutexRunning.unlock();
	owner->onFinishedRunning(this);
}

void Task::cancel() {
	_cancel = true;
}

bool Task::canceled() {
	return _cancel;
}

bool Task::isRunning() {
	QMutexLocker locker(&mutexRunning);
	return running;
}

QString Task::description() {
	return "";
}

float Task::progress() {
	return -1;
}
