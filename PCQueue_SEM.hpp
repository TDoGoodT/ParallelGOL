#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue

template <typename T>
class PCQueue
{
private:
    // Add your class memebers here
    queue<T> tasks;
    int read_count, write_count, q_size;
    Semaphore x, y;
    Semaphore read_sem, write_sem;

public:
    PCQueue():
        read_count(0),
        write_count(0),
        read_sem(1),
        write_sem(1),
        x(1), y(1){}
    ~PCQueue(){}

    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop(){

        read_sem.down();
        read_count++;
        if (read_count==1)
            write_sem.down();

        T res = tasks.front();
        q_size--;
        read_count--;
        if (read_count==0)
            write_sem.up();
        read_sem.up();

        return res;
    }

    bool try_pop(T* res){
        if(q_size > 0) {
            *res = pop();
            return true;
        }
        return false;
    }

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item){
        read_sem.down();
        write_sem.down();

        tasks.push(item);
        q_size++;

        write_sem.up();
        read_sem.up();
    }


    // Allows for producer to enter with *minimal delay* and push mutiple items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void multi_push(const vector<T>& items){
        //lock.writer_lock();
        for(auto item : items){
            tasks.push(item);
        }
        q_size += items.size();
        //lock.writer_unlock();
    }

    int size(){
        return q_size;
    }



};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif