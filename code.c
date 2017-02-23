#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>   

#define PRODUCERS 20
#define CONSUMERS 30
#define BUFFER_SIZE 1000
#define REQUEST 10000000

void *add_item();
void *remove_item();
void *append_buffer();
void *remove_buffer();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int buffer[BUFFER_SIZE] = {0}, tail = 0, head = 0, request = 0, success = 0;
clock_t timer1, timer2;

void main()
{
    timer1 = clock();   //Pull present cpu clock1
    
    pthread_t thread_producer[PRODUCERS];
    pthread_t thread_consumer[CONSUMERS];

/* Create independent threads each of which will used for producers */
    for(int i=0; i<PRODUCERS; i++)
        pthread_create(&thread_producer[i], NULL, append_buffer, NULL);

/* Create independent threads each of which will used for consumers */
    for(int i=0; i<CONSUMERS; i++)
        pthread_create(&thread_consumer[i], NULL, remove_buffer, NULL);

/* Wait till threads are complete before main continues. Unless we  */
/* wait we run the risk of executing an exit which will terminate   */
/* the process and all threads before the threads have completed.   */
    for(int i=0; i<CONSUMERS; i++) 
        pthread_join(thread_consumer[i], NULL);

    for(int i=0; i<PRODUCERS; i++)
        pthread_join(thread_producer[i], NULL);

    timer2 = clock();   //Pull present cpu clock2
    float elapsed = ((float)(timer2 - timer1) / CLOCKS_PER_SEC);

    printf("\n");
    printf("[+] Producers %d, Consumers %d\n", PRODUCERS, CONSUMERS);
    printf("[+] Buffer size %d\n", BUFFER_SIZE);
    printf("[+] Requests %d\n\n", request);
    printf("[+] Successfully consumed %d requests (%.1f%%)\n", success, (float)success * 100 / request);
    printf("[+] Elapsed Time %.2f s\n", elapsed);
    printf("[+] Throughput %.2f successful requests/s\n", (float)(success) / elapsed);

    exit(EXIT_SUCCESS);
}

void *add_item() {
    buffer[head++] = 1;
    head = head % BUFFER_SIZE;
}

void *remove_item() {
    buffer[tail++] = 0;
    tail = tail % BUFFER_SIZE;
}

void *append_buffer() {
    //printf("Append thread number %ld\n", pthread_self());
    while(request<REQUEST) {
        if(!pthread_mutex_trylock(&mutex) && request<REQUEST) {
            if(buffer[head] == 0) {
                add_item();
                request++;
                //printf(" + thread %ld append success\n", pthread_self());
            }
            else {
                //printf("Buffer overflow\n");
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);
}

void *remove_buffer() {
    //printf("Remove thread number %ld\n", pthread_self());
    while(request<REQUEST) {
        if(!pthread_mutex_trylock(&mutex) && request<REQUEST) {
            if(buffer[tail] == 1) {
                remove_item();
                success++;
                //printf(" - thread %ld remove success\n", pthread_self());
            }
            else {
                //printf("Buffer underflow\n");
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);
}
