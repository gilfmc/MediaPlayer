#ifndef LOADABLE_H
#define LOADABLE_H

#include <QtCore/QThreadPool>
#include <QtCore/QMutex>

class Loadable {
	public:
		void load();

	protected:
		virtual void onLoad() = 0;
		bool isLoaded();
		void unLoad();

	private:
		bool loaded = false;
		void concurrentLoad();
		
		QMutex mutex;
		
		class ConcurrentLoader : public QRunnable {
			public:
				ConcurrentLoader(Loadable * );
				virtual void run();
			
			private:
				Loadable * loadable;
		};
};

#endif // LOADABLE_H
