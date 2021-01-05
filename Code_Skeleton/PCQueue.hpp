#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue

template <typename T>
class PCQueue
{

public:

    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop(){
        read.down();
        T res = tasks.front();
        tasks.pop();
        return res;
    }

    bool try_pop(T* ret){
        if(read.get_val() > 0){
            read.down();
            *ret = tasks.front();
            tasks.pop();
            return true;
        }
        return false;
    }

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item){
        tasks.push(item);
        read.up();
    }


    // Allows for producer to enter with *minimal delay* and push mutiple items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void multi_push(const vector<T>& items){
        for(auto item : items){
            tasks.push(item);
        }
        for(int i = 0; i < items.size(); i++) read.up();
    }

    int size(){
        return tasks.size();
    }


private:
    // Add your class memebers here
    queue<T> tasks;
    Semaphore read;
    bool w_waits;
    bool r_inside;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif
