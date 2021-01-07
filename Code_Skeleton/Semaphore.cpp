#include "Semaphore.hpp"

Semaphore::Semaphore(): val(0) {
	pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
}
Semaphore::Semaphore(unsigned val): val(val) {
	pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
}
Semaphore::~Semaphore(){
	pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&lock);
}
void Semaphore::down(){ //Wait
	pthread_mutex_lock(&lock);
    while(val <= 0) {
        pthread_cond_wait(&cond,&lock);
    }
    --val;
    pthread_mutex_unlock(&lock);
} // Block untill counter >0, and mark - One thread has entered the critical section.
void Semaphore::up(){ //Signal
    pthread_mutex_lock(&lock);
    ++val;
    pthread_mutex_unlock(&lock);
    pthread_cond_broadcast(&cond);
} // Mark: 1 Thread has left the critical section

void Semaphore::up(int delta){ //Signal
    pthread_mutex_lock(&lock);
    val+=delta;
    pthread_mutex_unlock(&lock);
    for(int i = 0; i < delta; i++)
        pthread_cond_signal(&cond);
} // Mark: 1 Thread has left the critical section

int Semaphore::get_val(){
    return val;
}