#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//#include <iostream>
//#include <sys/queue.h>
//#include <mutex>

//using namespace std;

//queue<int> patients;
int seats = 0;
int size;

//mutex to ensure that only one patient is consulted at one time
pthread_mutex_t doct;

//mutex to ensure that the seats are occupied one after the other
pthread_mutex_t sts;

void *doctorState(void* param)
{
  int t = 1;
  int k = *(int *)param;
  while(t == 1)
  {
    if(k < 0 || k > size){
      printf("\nDoctor is sleeping");
    }
    else
    {
      int r = (rand() % 2) + 1;
      printf("\nPatient %d is getting treatment for %d seconds.", k, r);
      sleep(r);
      continue;
    }

  }
}

void *patientHandler(void* param)
{
    int t = 1;
    int j = *(int *)param;
    while(t == 1)
    {
      if(pthread_mutex_trylock(&doct) == 0)
      {
        pthread_mutex_lock(&sts);
        seats--;
        pthread_mutex_unlock(&sts);
        doctorState(j);
        pthread_mutex_unlock(&doct);
        continue;
      }
      else if(seats < 3)
      {
        pthread_mutex_lock(&sts);
        seats++;
        printf("\nPatient %d waiting. Seats Occupied: %d" ,j, seats);
        pthread_mutex_unlock(&sts);
      }
      else
      {
        int r = (rand() % j) + 1;
        printf("\n Patient %d is drinking coffee for %d seconds.", j, r);
        sleep(r);
      }
    }
}

int main()
{
    printf("\n");
    int i, threadIndex = 0;
    printf("Please enter number of patients: ");
    scanf("%d", size);

    // int A[size] = {0};

    //+1 for the doctor
    pthread_t threads[size+1];

    for (i = 0; i < size; i++)
    {

        pthread_create(&threads[threadIndex++], NULL, patientHandler, i);

    }

    pthread_create(&threads[threadIndex++], NULL, doctorState, 0);

    for(i = 0; i < size; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_join(threads[size+1], NULL);
    printf("\n\nProgram completed\n\n");
    system("PAUSE");
    return 0;
}
