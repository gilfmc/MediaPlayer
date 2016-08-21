#include "Loadable.h"

void Loadable::load() {
	if(loaded) return;
	
	QThreadPool::globalInstance()->start(new ConcurrentLoader(this));
}

void Loadable::concurrentLoad() {
	mutex.lock();
	if(loaded) return;
	onLoad();
	loaded = true;
	mutex.unlock();
}

bool Loadable::isLoaded() {
	return loaded;
}

void Loadable::unLoad() {
	loaded = false;
}

Loadable::ConcurrentLoader::ConcurrentLoader(Loadable * loadable) : loadable(loadable) { }

void Loadable::ConcurrentLoader::run() {
	loadable->concurrentLoad();
}
