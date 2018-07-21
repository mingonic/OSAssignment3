#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <sys/queue.h>
#include <mutex>

using namespace std;

queue<int> patients;
int seats = 0;

//mutex to ensure that only one patient is consulted at one time
mutex doct;

//mutex to ensure that the seats are occupied one after the other
mutex sts;
