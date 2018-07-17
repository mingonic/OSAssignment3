#include <stdio.h>
//#include "stdafx.h"
#include <iostream>
#include <pthread.h>
#include <queue>
#include <mutex>

using namespace std;

queue<int> patients;
int seats = 0;

//mutex to ensure that only one patient is consulted at one time
mutex doct;

//mutex to ensure that the seats are occupied one after the other
mutex sts;

void doctorState()
{
    while (true)
    {
        if (!patients.empty())
        {
            printf("\nPatient %d is getting treatment.", patients.front());
            // cout << "\nPatient " << patients.front()  << " is getting treatment";
            patients.pop();
            break;
        }
        else
        {
            printf("\nDoctor is sleeping");
            //count << "\n Doctor is sleeping";
            break;
        }
    }
}

void *patientHandler(void *i)
{
    while (true)
    {
        if (doct.try_lock())
        {
            sts.lock();
            seats--;
            sts.unlock();
            doctorState();
            doct.unlock();
            break;
        }
        else if (seats < 3)
        {
            sts.lock();
            printf("\n Patient %d waiting. Seats Occupied", i);
            seats++;
            //cout << "\n Patient " << i << " waiting. Seats Occupied " << ++seats;
            sts.unlock();
        }
        else
        {
            printf("\n Patient %d drinking coffee for %d seconds.", i, i);
            //cout << "\n Patient " << i << "drinking coffee for " << i << " seconds";
            this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

int main()
{
    pthread thread1, thread2, thread3, thread4, threadD;
    int i1, i2, i3, i4, iD;

    i1 = pthread_create( &thread1, NULL, patientHandler, (void*) 1);
    i2 = pthread_create( &thread2, NULL, patientHandler, (void*) 2);
    i3 = pthread_create( &thread3, NULL, patientHandler, (void*) 3);
    i4 = pthread_create( &thread4, NULL, patientHandler, (void*) 4);

    iD = pthread_create( &threadD, NULL, doct, doctorState);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    pthread_join(threadD, NULL);

}
