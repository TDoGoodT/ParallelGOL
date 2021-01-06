#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include "Headers.hpp"
#include <pthread.h>

typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;

// Synchronization Warm up 
class Semaphore {
public:
	Semaphore(); // Constructs a new semaphore with a counter of 0
	Semaphore(unsigned val); // Constructs a new semaphore with a counter of val
	~Semaphore();


    void up(); // Mark: 1 Thread has left the critical section
    void up(int); // Mark: 1 Thread has left the critical section
	void down(); // Block untill counter >0, and mark - One thread has entered the critical section.
	int get_val();
private:
	// TODO 
	int val; //counting resources if val>0 and waiting tasks if val<0
	cond_t cond;
	mutex_t lock;	
};

#endif
