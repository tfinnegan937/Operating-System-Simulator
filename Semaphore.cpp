//
// Created by tim on 10/28/19.
//

#include "Semaphore.h"
Semaphore::Semaphore(){

}
Semaphore::Semaphore(pthread_mutex_t * mutex_in, int count){
    total_count = count;
    cur_count = total_count;
    mutex = mutex_in;

    pthread_mutex_init(mutex, NULL);
}

int Semaphore::getCount(){
    return total_count - cur_count;
}

Semaphore::~Semaphore(){
    pthread_mutex_destroy(mutex);
}

void Semaphore::init(pthread_mutex_t * mutex_in, int count){
    total_count = count;
    cur_count = total_count;
    mutex = mutex_in;

    pthread_mutex_init(mutex, NULL);
}

void Semaphore::wait(){
    if(cur_count > 0){
        cur_count--;
    }
    if(cur_count == 0){
        pthread_mutex_lock(mutex);
    }
}

void Semaphore::signal(){
    bool unlock = false;
    if(cur_count == 0){
        unlock = true;
    }

    cur_count++;
    if(unlock){
        pthread_mutex_unlock(mutex);
    }
}