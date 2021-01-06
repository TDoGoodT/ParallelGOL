#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue
template <typename T>
class PCQueue
{

public:
    PCQueue(){
        pthread_mutex_init(&m, NULL);
    }
    ~PCQueue(){
        pthread_mutex_destroy(&m);
    }

    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop(){
        queue_size.down();
        pthread_mutex_lock(&m);
        T res = tasks.front();
        tasks.pop();
        pthread_mutex_unlock(&m);
        return res;
    }

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item){
        pthread_mutex_lock(&m);
        tasks.push(item);
        queue_size.up();
        pthread_mutex_unlock(&m);
    }


    // Allows for producer to enter with *minimal delay* and push mutiple items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void multi_push(const vector<T>& items){
        pthread_mutex_lock(&m);
        for(auto item : items){
            tasks.push(item);
            queue_size.up();
        }
        pthread_mutex_unlock(&m);
    }

    int size(){
        return queue_size.get_val();
    }


private:
    // Add your class memebers here
    queue<T> tasks;
    Semaphore queue_size;
    mutex_t m;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif