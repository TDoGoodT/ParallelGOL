#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue

class RWLock{
public:
    RWLock(): w_waiting(false), r_inside(false), w_inside(false){
        pthread_cond_init(&r_allowed, NULL);
        pthread_cond_init(&w_allowed, NULL);
        pthread_mutex_init(&glb_lock, NULL);
    }
    void reader_lock(){
        pthread_mutex_lock(&glb_lock);
        while(w_inside || w_waiting || r_inside)
            pthread_cond_wait(&r_allowed, &glb_lock);
        r_inside = true;
        pthread_mutex_unlock(&glb_lock);
    }
    void reader_unlock(){
        pthread_mutex_lock(&glb_lock);
        r_inside = false;
        pthread_cond_signal(&w_allowed);
        pthread_cond_signal(&r_allowed);
        pthread_mutex_unlock(&glb_lock);
    }
    void writer_lock(){
        pthread_mutex_lock(&glb_lock);
        w_waiting = true;
        while(r_inside)
            pthread_cond_wait(&w_allowed, &glb_lock);
        w_waiting = false;
        w_inside = true;
        pthread_mutex_unlock(&glb_lock);
    }

    void writer_unlock(){
        pthread_mutex_lock(&glb_lock);
        w_inside = false;
        pthread_cond_broadcast(&r_allowed);
        pthread_mutex_unlock(&glb_lock);
    }
private:
    bool w_waiting, r_inside, w_inside;
    cond_t r_allowed, w_allowed;
    mutex_t glb_lock;

};

template <typename T>
class PCQueue
{

public:
    PCQueue(){}
    ~PCQueue(){}

    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop(){
        queue_size.down();
        lock.reader_lock();
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
        queue_size.up();
        lock.writer_unlock();
    }


    // Allows for producer to enter with *minimal delay* and push mutiple items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void multi_push(const vector<T>& items){
        lock.writer_lock();
        for(auto item : items){
            tasks.push(item);
            queue_size.up();
        }
        lock.writer_unlock();
    }

    int size(){
        //lock.reader_lock();
        //int res = tasks.size();
        return queue_size.get_val();
        //lock.reader_unlock();
        //return queue_size.get_val();
    }


private:
    // Add your class memebers here
    queue<T> tasks;
    RWLock lock;
    Semaphore queue_size;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif