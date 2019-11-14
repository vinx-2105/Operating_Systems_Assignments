/*
Author: Vineet Madan
Date: 14 Nov 2019
Purpose: Lab 8 of CS 303 Operating Systems
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include "buffer.h"


//function prototypes
int insert_item(buffer_item item);
int remove_item(buffer_item *item);
void init_sems();
void *producer(void *param);
void *consumer(void *param);


//global vars
buffer_item buffer[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t full, empty;
int count, in, out;

int main(int argc, char *argv[]){
    //there should be 4 args
    if(argc!=4){
        printf("Invalid number of arguments\n");
        exit(1);
    }

    //get the cl args
    int sleep_time = atoi(argv[1]);
    int num_producer = atoi(argv[2]);
    int num_consumer = atoi(argv[3]);

    srand(time(NULL));

    //init pthread_mutex
    pthread_mutex_init(&mutex, NULL);

    //init semaphores
    init_sems();

    count=0;
    in=0;
    out=0;

    int i;
    //create arrays of producer and consumer threads
    pthread_t consumer_threads[num_consumer];
    pthread_t producer_threads[num_producer];

    for(i=0; i<num_consumer; i++){
        pthread_create(&consumer_threads[i], NULL, consumer, NULL);
    }
    for(i=0;i<num_producer; i++){
        pthread_create(&producer_threads[i], NULL, producer, NULL);
    }

    // Sleep before terminating
  	sleep(sleep_time);
  	return 0;
}

//insert item into buffer
//returns 0 if successful and, -1 otherwise
int insert_item(buffer_item item){
    int result;
    //the init part
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    //the critical section
    if(count != BUFFER_SIZE){
        buffer[in] = item;
        //increment in
        in=(in+1)%BUFFER_SIZE;
        count++;
        result = 0;
    }
    else{
        result=-1;
    }
    pthread_mutex_unlock(&mutex);
    sem_post(&full);
    return result;
}

//remove an object from the buffer and place it in the item pointer
//returns 0 if successful, -1 otherwise
int remove_item(buffer_item *item){
    int result;

    //wait for the full...buffer has some items
    sem_wait(&full);

    //lock the mutex
    pthread_mutex_lock(&mutex);

    //critical code...remove the item
    if(count!=0){
        //place the item
        *item = buffer[out];
        out = (out+1)%BUFFER_SIZE;
        count--;
        result=0;
    }
    else{
        result = -1;
    }
    //unlock mutex
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);

    return result;
}

void init_sems(){
    //value of empty is buffer_size initital
    sem_init(&empty, 0, BUFFER_SIZE);
    //value of full is 0 initially
    sem_init(&full, 0, 0);
}

//consumer takes items from the buffer
void *consumer(void *param){
    buffer_item bi;
    while(1){
        //sleep for some random time
        sleep(rand()%5+1);
        if(remove_item(&bi)==-1){
            printf("An error occurred\n");
        }
        else{
            printf("consumed item: %d, count of items: %d\n", bi, count);
        }
    }
}

//producer adds items to the buffer
void *producer(void *param){
    buffer_item bi;//init to a random number
    while(1){
        sleep(rand()%5+1);

        bi = rand()%100+1;

        //insert this item bi into the buffer
        if(insert_item(bi)==-1){
            printf("An error occurred\n");
        }
        else{
            printf("produced item: %d, count of items: %d\n", bi, count);
        }
    }
}