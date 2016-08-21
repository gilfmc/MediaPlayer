#ifndef TASK_H
#define TASK_H

#include <QtCore/QMutex>
#include <QtCore/QThreadPool>
#include <QtCore/QWaitCondition>

class Task;

class TaskOwner {
	public:
		virtual void onFinishedRunning(Task * task) = 0;
};

class Task : private QRunnable {
	public:
		virtual QString name() = 0;
		virtual QString description();
		virtual float progress();

		bool isRunning();

		void cancel();
		bool canceled();

		void run(TaskOwner * owner);
		void run();
		virtual void onRun() = 0;

	private:
		QMutex mutexRunning;
		bool running = false;
		
		bool _cancel = false;
		
		TaskOwner * owner;
};

#endif // TASK_H
