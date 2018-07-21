#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE 3
#define N 5
#define PRODUCER_LOOPS 2

typedef int buffer_t;
buffer_t buffer[SIZE];
int seats_occupied;

pthread_mutex_t doctor_mutex;
pthread_mutex_t buffer_mutex;
/* initially buffer will be empty.  full_sem
   will be initialized to buffer SIZE, which means
   SIZE number of producer threads can write to it.
   And empty_sem will be initialized to 0, so no
   consumer can read from buffer until a producer
   thread posts to empty_sem */
sem_t full_sem;  /* when 0, buffer is full */
sem_t empty_sem; /* when 0, buffer is empty. Kind of
                    like an index for the buffer */

/* sem_post algorithm:
    mutex_lock  sem_t->mutex
    sem_t->value++
    mutex_unlock sem_t->mutex

   sem_wait algorithn:
    mutex_lock sem_t->mutex
    while (sem_t->value > 0) {
        mutex_unlock sem_t->mutex
        sleep... wake up
        mutex_lock sem_t->mutex
    }
    sem_t->value--
    mutex_unlock sem_t->mutex
*/
void dequeuebuffer(buffer_t value);
void patient_handler(int id);
void insertbuffer(buffer_t value);
void *producer(void *tid);
void *examination(void *tid);

int main(int argc, int **argv) {
    seats_occupied = 0;

    pthread_mutex_init(&buffer_mutex, NULL);
	pthread_mutex_init(&doctor_mutex, NULL);
    sem_init(&full_sem, // sem_t *sem
             0, // int pshared. 0 = shared between threads of process,  1 = shared between processes
             SIZE); // unsigned int value. Initial value
    sem_init(&empty_sem,
             0,
             0);
    /* full_sem is initialized to buffer size because SIZE number of
       producers can add one element to buffer each. They will wait
       semaphore each time, which will decrement semaphore value.
       empty_sem is initialized to 0, because buffer starts empty and
       consumer cannot take any element from it. They will have to wait
       until producer posts to that semaphore (increments semaphore
       value) */
    pthread_t thread[N];
	pthread_t doctor_thread;
    int thread_numb[N];
    int i, j;

	j = 100;
	if (pthread_create(&doctor_thread, NULL, examination, (void *) &j)) {
		printf("error creating doctor thread");
	}
    for (i = 0; i < N; ) {
        thread_numb[i] = i;
        pthread_create(thread + i, // pthread_t *t
                       NULL, // const pthread_attr_t *attr
                       producer, // void *(*start_routine) (void *)
                       thread_numb + 1);  // void *arg
        i++;
    }

    for (i = 0; i < N; i++)
        pthread_join(thread[i], NULL);
	pthread_join(doctor_thread, NULL);

    pthread_mutex_destroy(&buffer_mutex);
	pthread_mutex_destroy(&doctor_mutex);
    sem_destroy(&full_sem);
    sem_destroy(&empty_sem);

    return 0;
}

void insertbuffer(buffer_t value) {
    if (seats_occupied < SIZE) {
        buffer[seats_occupied++] = value;
		printf("Patient %d waiting. Seats are occupied = %d.\n", value, seats_occupied - 1);
		patient_handler(value);
    } else {
        printf("Patient %d drinking coffee for %d seconds.\n", rand() %10);
    }
}

void dequeuebuffer(buffer_t value) {
    if (seats_occupied > 0) {
        buffer[--seats_occupied]; // seats_occupied-- would be error!
		printf("Patient %d being treated.", value);
    } else {
        printf("No patents in waiting room. Doctor is going to sleep");
    }
}


void *producer(void *tid) {
    int thread_numb =  *(int *)tid;
    buffer_t value;
    int i=0;
    while (i++ < PRODUCER_LOOPS) {
        sleep(rand() % 10);
        value = thread_numb;
        sem_wait(&full_sem); // sem=0: wait. sem>0: go and decrement it
        /* possible race condition here. After this thread wakes up,
           another thread could aqcuire mutex before this one, and add to list.
           Then the list would be full again
           and when this thread tried to insert to buffer there would be
           a buffer overflow error */
        pthread_mutex_lock(&buffer_mutex); /* protecting critical section */
		insertbuffer(value);
        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&empty_sem); // post (increment) emptybuffer semaphore
        // printf("Producer %d added %d to buffer\n", thread_numb, value);
    }
    pthread_exit(0);
}

void patient_handler(int id) {
	int i=0;
    while (i++ < PRODUCER_LOOPS) {
		if (pthread_mutex_trylock(&doctor_mutex) == 0) {
			sem_wait(&empty_sem);
			pthread_mutex_lock(&buffer_mutex);
			dequeuebuffer(id);
			pthread_mutex_unlock(&buffer_mutex);
			sem_post(&full_sem); // post (increment) fullbuffer semaphore
			pthread_mutex_unlock(&doctor_mutex);
		}
        // printf("Consumer %d dequeue %d from buffer\n", thread_numb, value);
   }
   pthread_exit(0);
}

void *examination(void *tid) {
    // int thread_numb = 1500;
	buffer_t value =  *(int *)tid;
	patient_handler(value);
}
