#include <stdio.h>
//#include "stdafx.h"
#include <iostream>
#include <thread>
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

            cout << "\nPatient " << patients.front()  << " is getting treatment.";
            patients.pop();
            break;
        }
        else
        {
            cout << "\nDoctor is sleeping.";
            break;
        }
    }
}

void patientHandler(int i)
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
            cout << "\nPatient " << i << " waiting. Seats Occupied. " << ++seats <<" seats full.";
            sts.unlock();
        }
        else
        {
            cout << "\nPatient " << i << " drinking coffee for " << i << " seconds.";
            this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

int main()
{
    printf("\n");
    thread p[10];
    for (int i = 0; i < 10; i++)
    {
        p[i] = std::thread(patientHandler, i);
        patients.push(i);
    }
    thread doct(doctorState);

    for(int i = 0; i < 10; i++)
    {
        p[i].join();
    }
    doct.join();
    printf("\n\n");
}
