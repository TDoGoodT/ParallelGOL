#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue

class RWLock{
public:
    RWLock():
            r_inside(0), w_inside(0), w_waiting(false){
        pthread_cond_init(&r_allowed, NULL);
        pthread_cond_init(&w_allowed, NULL);
        pthread_mutex_init(&glb_lock, NULL);
    }
    void reader_lock(){
        pthread_mutex_lock(&glb_lock);
        while(w_inside > 0)
            pthread_cond_wait(&r_allowed, &glb_lock);
        r_inside++;
        pthread_mutex_unlock(&glb_lock);
    }
    void reader_unlock(){
        pthread_mutex_lock(&glb_lock);
        r_inside--;
        if(!r_inside){
            pthread_cond_signal(&w_allowed);
        }
        pthread_mutex_unlock(&glb_lock);
    }
    void writer_lock(){
        pthread_mutex_lock(&glb_lock);
        w_inside++;
        pthread_mutex_unlock(&glb_lock);

    }
    void writer_unlock(){
        pthread_mutex_lock(&glb_lock);
        w_inside--;
        if(w_inside == 0){
            pthread_cond_broadcast(&r_allowed);
            pthread_cond_signal(&w_allowed);
        }
        pthread_mutex_unlock(&glb_lock);
    }
private:
    int r_inside, w_inside;
    bool w_waiting; //assumes single writer
    cond_t r_allowed, w_allowed;
    mutex_t glb_lock;

};

template <typename T>
class PCQueue
{

public:

    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop(){
        lock.reader_lock();
        while(tasks.empty()) {}
        T res = tasks.front();
        tasks.pop();
        lock.reader_unlock();
        return res;
    }

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item){
        lock.writer_lock();
        tasks.push(item);
        lock.writer_unlock();
    }


    // Allows for producer to enter with *minimal delay* and push mutiple items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void multi_push(const vector<T>& items){
        lock.writer_lock();
        for(auto item : items){
            tasks.push(item);
        }
        lock.writer_unlock();
    }

    int size(){
        return tasks.size();
    }


private:
    // Add your class memebers here
    queue<T> tasks;
    RWLock lock;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif
