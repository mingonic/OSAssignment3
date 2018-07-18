#include <stdio.h>
//#include "stdafx.h"
#include <iostream>
#include <pthread.h>
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
            int r = (rand() % 10) + 1;
            cout << "\nPatient " << patients.front()  << " is getting treatment for " << r << " seconds.";
            this_thread::sleep_for(std::chrono::seconds(r));
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
            doct.lock();
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
            seats++;
            cout << "\nPatient " << i << " waiting. Seats Occupied: " << seats;
            sts.unlock();
        }
        else
        {
            int r = (rand() % i) + 1;
            cout << "\nPatient " << i << " drinking coffee for " << r << " seconds.";
            this_thread::sleep_for(std::chrono::seconds(r));
        }
    }
}

int main()
{
    printf("\n");
    int size, i;
    cout << "Please enter number of patients: ";
    cin >> size;

    thread doct(doctorState);

    thread p[size];
    for (i = 0; i < size; i++)
    {
        //p[i] = std::thread(patientHandler, i);
        p[i] = thread(patientHandler, i+1);
        patients.push(i);
    }



    for(i = 0; i < size; i++)
    {
        p[i].join();
    }
    doct.join();
    printf("\n\nProgram completed\n\n");
}
