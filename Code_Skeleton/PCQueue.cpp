#include "PCQueue.hpp"

template <typename T>
PCQueue<T>::PCQueue(){
	pthread_mutex_init(&m, NULL);
}

template <typename T>
PCQueue<T>::~PCQueue(){
    pthread_mutex_destroy(&m);
}

template <typename T>
T PCQueue<T>::pop(){
	queue_size.down();
	pthread_mutex_lock(&m);
	tasks.pop();
	pthread_mutex_unlock(&m);
}

template <typename T>
void PCQueue<T>::push(const T& item){ 
	pthread_mutex_lock(&m);
	tasks.push(item);
	pthread_mutex_unlock(&m);
	queue_size.up();
}
