#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QtCore/QList>

#include "business/Task.h"

class TaskManager : public QObject, private TaskOwner {
		Q_OBJECT

	public:
		TaskManager();

		void run(Task * task);

		bool hasRunningTasks();
		bool hasTaskWithName(const QString & name);
		
		void onFinishedRunning(Task * task);

		~TaskManager();

	private:
		QList<Task*> tasks;

		QMutex mutex;
};

#endif // TASKMANAGER_H
