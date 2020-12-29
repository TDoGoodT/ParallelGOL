#include "PCQueue.hpp"
#include "Semaphore.hpp"

Semaphore sem();

template <typename T>
PCQueue<T>::pop(){

}

template <typename T>
PCQueue<T>::push(const T& item){ //assume single Producer => no need to use sync.
	tasks.push(item);
}
