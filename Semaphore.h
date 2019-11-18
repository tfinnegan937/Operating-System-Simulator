//
// Created by tim on 10/28/19.
//

#ifndef SIM03_TIMFINNEGAN_SEMAPHORE_H
#define SIM03_TIMFINNEGAN_SEMAPHORE_H
#include <pthread.h>
using namespace std;
class Semaphore {
private:
    pthread_mutex_t * mutex;
    int cur_count;
    int total_count;
public:
    Semaphore();
    Semaphore(pthread_mutex_t * mutex_in, int count);
    ~Semaphore();

    void init(pthread_mutex_t * mutex_in, int count);
    void wait();
    void signal();

    int getCount();
};


#endif //SIM02_TIMFINNEGAN_SEMAPHORE_H
