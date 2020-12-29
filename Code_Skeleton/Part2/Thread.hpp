#ifndef __THREAD_H
#define __THREAD_H
#include "Headers.hpp"
class Thread
{
public:
	Thread(uint thread_id) 
	{
		// Only places thread_id 
	} 
	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	bool start()
	{
	}

	/** Will not return until the internal thread has exited. */
	void join()
	{
	}

	/** Returns the thread_id **/
	uint thread_id()
	{

	}
protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
	virtual void thread_workload() = 0;
	uint thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use

private:
	static void * entry_func(void * thread) { ((Thread *)thread)->thread_workload(); return NULL; }
	pthread_t m_thread;
};

#endif
