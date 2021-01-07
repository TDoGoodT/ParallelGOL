//
// Created by Snir Bachar on 07/01/2021.
//

#ifndef HW3_OS_BARRIER_HPP
#define HW3_OS_BARRIER_HPP
#include "Semaphore.hpp"

class Barrier {
private:
    mutex_t mutex;
    Semaphore barrier;
    uint count;
    uint num_threads;
public:
    Barrier(uint threads = 0):
        count(0),
        num_threads(threads){
        pthread_mutex_init(&mutex,NULL);
    }

    void block(){
        pthread_mutex_lock(&mutex);
        int res = ++count;
        pthread_mutex_unlock(&mutex);
        if(res == num_threads) {
            barrier.up();
        }
        barrier.down();
        barrier.up();
    }
};


#endif //HW3_OS_BARRIER_HPP
