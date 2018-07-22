#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

// The maximum number of patient threads.
#define PATIENT_MAX 20
sem_t wait_room;
sem_t doctor_mutex;
sem_t contact_doctor;
sem_t d_sleep;
sem_t treating;

int finished = 0;
void *doctor(void *);
void *patient_handler(void *num);
int getSeats();

int main() {
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

    //Semaphore to ensure only one person in examination room at a time
    sem_init(&doctor_mutex, 0, 1);

    //Semaphore making contact with the doctor in the examination room
    sem_init(&contact_doctor, 0, 0);

    //Semaphor checking if doctor is sleeping
    sem_init(&d_sleep, 0, 0);
    
    //Semaphore making sure patient is in treating room until finished treatment
    sem_init(&treating, 0, 0);

    /*Thread Creation*/
    //doctor thread
    pthread_create(&doctor_thread, NULL, doctor, NULL);

    //Generating a random number using the time as the seed value
    srand(time(NULL));

    //Patient threads
    for (i = 0; i < numPatients; i++) {
        pthread_create(&patient_threads[i], NULL, patient_handler, (void *)&patient_ids[i]);
    }

    //Joining each thread back, to finish
    for (i=0; i<numPatients; i++) {
        pthread_join(patient_threads[i], NULL);
    }
    
    //Marking program as finished
    finished = 1;

    //Doctor needs to wake up and exit
    sem_post(&contact_doctor);
    sem_post(&d_sleep);
    pthread_join(doctor_thread, NULL);

    //Destroying semaphors
    sem_destroy(&wait_room);
    sem_destroy(&doctor_mutex);
    sem_destroy(&contact_doctor);
    sem_destroy(&d_sleep);
    sem_destroy(&treating);

    return 0;
}

void *patient_handler(void *id) {
    int value = *(int *)id;
    int i = 0;

    int r = rand() % 10;  

    //Each patient will see the doctor 3 times
    while (i < 3) {
		printf("Patient %d drinking coffee for %d seconds.\n", value, r);
		sleep(r);

        //Patient entering the waiting room
        while (sem_trywait(&wait_room) != 0) {
            r = rand() % 10; 
            printf("Waiting room is full. Patient %d drinking coffee for %d seconds.\n", value, r);
            sleep(r);

        }
        printf("Patient %d waiting. Seats occupied = %d.\n", value, getSeats());

        //Patient entering examination room, if there are no other patients inside
        sem_wait(&doctor_mutex);

        //Patient leaving waiting room, so seat available
        sem_post(&wait_room);

        //Patient enters examination room and makes contact with the doctor
        sem_post(&contact_doctor);
	
        //Patient sees if doctor is sleeping. If so, wakes up the doctor. 
        int s;
        sem_getvalue(&d_sleep, &s);
        if (s == 0) {
            printf("Patient %d waking the doctor. Seats occupied = %d.\n", value, getSeats());
            sem_post(&d_sleep);
        }

        //Patient being treated.
        printf("Patient %d is getting treatment.\n", value);
		sem_wait(&treating);

		//Patient has finished being treated and exits room.
		sem_post(&doctor_mutex);
		printf("Patient %d leaving examination room.\n", value);

        //Patient goes to treatment for 3 times in total
		i++;
    }
    //This signals that the patient has completed all 3 rounds of treatment, and is leaving the hospital. 
    printf("Patient %d has fully been treated. Leaving the hospital. YAY!.\n", value);
}

void *doctor(void *blank) {
    int r; 
    printf("The doctor is sleeping.\n");
    while (!finished) {
        //Patient enters the examination room and makes contact with the doctor
        sem_wait(&contact_doctor);	

        //Making sure doctor is awake
        sem_wait(&d_sleep);
        
        //This code will not run last loop
        if (!finished) {
           	r = rand() % 10; 

           	printf("Doctor treating patient for %d seconds. Seats occupied = %d.\n", r, getSeats());
           	sleep(r);
           	printf("Doctor has finished treating the patient.\n");

	  	    //Doctor says patient has finished being treated and patient is released
          	sem_post(&treating);

            int s;
            sem_getvalue(&d_sleep, &s);

            //Checking if there are more patients in waiting room, and if doctor is awake
            //Otherwise the doctor will go back to sleep.
            if (getSeats() != 0 && s == 0) {
                printf("Doctor is checking for next patient.\n");
                
                //Makes sure doctor stays awake using semaphor
                sem_post(&d_sleep);
            } else {
                printf("Doctor is going back to sleep.\n");
            }
        }
    }
    printf("No more patients! Doctor is done for the day.\n");
}

int getSeats () {
    //Gets the number of seats that are available in the waiting room
    int seatNum;
    sem_getvalue(&wait_room, &seatNum);
    return 3-seatNum;
}
