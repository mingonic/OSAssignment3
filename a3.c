#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

// The maximum number of customer threads.
#define PATIENT_MAX 20
sem_t wait_room;
sem_t doctor_mutex;
sem_t doctor_sleep;
sem_t treating;

int finished = 0;
void *doctor(void *);
void *patient_handler(void *num);
int getSeats();

int main(void) {
    //Thread for doctor and thread array for patients
    pthread_t doctor_thread;
    pthread_t patient_threads[PATIENT_MAX];
    // Number of patients
    int numPatients;

    printf("Please enter number of patients: ");
    scanf("%d", &numPatients);

    while (numPatients > PATIENT_MAX) {
        printf("Please enter patients less than %d: ", PATIENT_MAX);
        scanf("%d", &numPatients);
    }

    int i;
    int patient_ids[numPatients];

    //Array that keeps track of thread number, passed into function during patient thread creation;
    for (i = 0; i < numPatients; i++) {
        patient_ids[i] = i;
    }

    /*Semaphore Initialization*/
    //Semaphore for number of seats in waiting room (3)
    sem_init(&wait_room, 0, 3);
    //Binary Semaphore (mutex) to ensure only one person in examination room
    sem_init(&doctor_mutex, 0, 1);
    //Semaphore making sure doctor is sleeping
    sem_init(&doctor_sleep, 0, 0);
    //Semaphore making sure patient is in treating room until finished treatment
    sem_init(&treating, 0, 0);

    /*Thread Creation*/
    //doctor thread
    pthread_create(&doctor_thread, NULL, doctor, NULL);

    //Generating a random number using the time as the seed value
    srand48(time(NULL));

    //Patient threads
    for (i = 0; i < numPatients; i++) {
        pthread_create(&patient_threads[i], NULL, patient_handler, (void *)&patient_ids[i]);
    }

    //Joining each thread back, to finish
    for (i=0; i<numPatients; i++) {
        pthread_join(patient_threads[i], NULL);
    }
    
    finished = 1;

    //Doctor needs to wake up and exit
    sem_post(&doctor_sleep);
    pthread_join(doctor_thread, NULL);
}

void *patient_handler(void *id) {
    int value = *(int *)id;
    int i = 0;

    int r = rand() % 10;  
    //Each patient will see the doctor 3 times
    while (i < 1) {
		printf("Patient %d drinking coffee for %d seconds.\n", value, r);
		sleep(r);

        //Patient entering the waiting room
        while (sem_trywait(&wait_room) != 0) {
            r = rand() % 10; 
            printf("Doctor currently unavailable. Patient %d drinking coffee for %d seconds.\n", value, r);
            sleep(r);

        }

        // sem_wait(&wait_room);
        printf("Patient %d waiting. Seats occupied = %d.\n", value, getSeats());

        //Patient entering examination room, if seat available
        sem_wait(&doctor_mutex);
        //Patient leaving waiting room, so seat available
        sem_post(&wait_room);

        printf("Patient %d waking the doctor. Seats occupied = %d.\n", value, getSeats());
        sem_post(&doctor_sleep);
	
        //Patient being treated.
        printf("Patient %d is getting treatment.\n", value);
		sem_wait(&treating);

		//Patient has finished being treated
		sem_post(&doctor_mutex);
		printf("Patient %d leaving examination room.\n", value);
		i++;
    }
    printf("Patient %d has fully been treated. Going home.\n", value);
}

void *doctor(void *blank) {
    int r; 

    while (!finished) {
        //Doctor sleep
        printf("The doctor is sleeping.\n");
        sem_wait(&doctor_sleep);	

        if (!finished) {
           	r = rand() % 10; 

           	printf("Doctor treating patient for %d seconds. Seats occupied = %d.\n", r, getSeats());
           	sleep(r);
           	printf("Doctor has finished treating the patient.\n");

	  	    //Doctor says patient has finished being treated and patient is released
          	sem_post(&treating);

        }
    }
    printf("Doctor is done for the day.\n");
}

int getSeats () {
    int seatNum;
    sem_getvalue(&wait_room, &seatNum);
    return 3-seatNum;
}
