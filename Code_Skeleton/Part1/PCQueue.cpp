#include "PCQueue.hpp"

template <typename T>
PCQueue::PCQueue(){
	pthread_mutex_init(&m, NULL);
}

template <typename T>
Semaphore::~PCQueue(){
    pthread_mutex_destroy(&m);
}

template <typename T>
PCQueue<T>::pop(){
	queue_size.down();
	pthread_mutex_lock(&m);
	tasks.pop();
	pthread_mutex_unlock(&m);
}

template <typename T>
PCQueue<T>::push(const T& item){ 
	pthread_mutex_lock(&m);
	tasks.push(item);
	pthread_mutex_unlock(&m);
	queue_size.up();
}
