#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_CUSTOMERS 120
#define CHAIRS 7
#define CUT_DELAY 2500000UL
#define SHOPPING_DELAY 700000UL
#define ARRIVAL_DELAY 50000UL

static sem_t mutex;
static sem_t customers;
static sem_t stylist_ready;

static int waiting = 0;
static int haircuts_given = 0;
static int salon_full_score = 0;
static int salon_empty_count = 0;
static int shopping_trips = 0;

static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

static void sem_down(sem_t *sem)
{
    while (sem_wait(sem) != 0) {
        if (errno != EINTR) {
            die("sem_wait");
        }
    }
}

static void sem_up(sem_t *sem)
{
    if (sem_post(sem) != 0) {
        die("sem_post");
    }
}

static void delay_loop(unsigned long bound)
{
    volatile unsigned long i;

    for (i = 0; i < bound; i++) {
    }
}

static void print_chairs_locked(void)
{
    int i;

    printf("|");
    for (i = 0; i < CHAIRS; i++) {
        printf(" %d |", i < waiting ? 1 : 0);
    }
    printf(" => %d\n", waiting);
    printf("Given haircuts = %d\n", haircuts_given);
    printf("Salon full = %d times\n", salon_full_score);
    printf("Salon empty = %d times\n\n", salon_empty_count);
}

static void *stylist_thread(void *arg)
{
    (void)arg;

    while (1) {
        sem_down(&mutex);
        if (waiting == 0 && haircuts_given < NUM_CUSTOMERS) {
            salon_empty_count++;
            printf("[stylist] no customers are waiting; stylist goes to sleep.\n");
            print_chairs_locked();
        }
        sem_up(&mutex);

        sem_down(&customers);

        sem_down(&mutex);
        waiting--;
        printf("[stylist] calls the next customer from the waiting chairs.\n");
        print_chairs_locked();
        sem_up(&stylist_ready);
        sem_up(&mutex);

        delay_loop(CUT_DELAY);

        sem_down(&mutex);
        haircuts_given++;
        printf("[stylist] finished haircut %d of %d.\n\n",
               haircuts_given, NUM_CUSTOMERS);
        if (haircuts_given == NUM_CUSTOMERS) {
            printf("[stylist] all customers have received haircuts; shop closes.\n");
            sem_up(&mutex);
            break;
        }
        sem_up(&mutex);
    }

    return NULL;
}

static void *customer_thread(void *arg)
{
    int id = (int)(intptr_t)arg;
    int attempts = 0;

    delay_loop((unsigned long)(id % 11) * ARRIVAL_DELAY);

    while (1) {
        attempts++;
        sem_down(&mutex);

        if (waiting < CHAIRS) {
            waiting++;
            if (waiting == CHAIRS) {
                salon_full_score += CHAIRS;
                printf("[customer %03d] takes the last waiting chair; salon is full.\n",
                       id);
            } else {
                printf("[customer %03d] sits in a waiting chair.\n", id);
            }
            print_chairs_locked();

            sem_up(&customers);
            sem_up(&mutex);

            sem_down(&stylist_ready);

            sem_down(&mutex);
            printf("[customer %03d] moves to the styling chair after %d attempt(s).\n\n",
                   id, attempts);
            sem_up(&mutex);
            break;
        }

        shopping_trips++;
        printf("[customer %03d] finds the salon full and goes shopping "
               "(trip %d).\n", id, shopping_trips);
        print_chairs_locked();
        sem_up(&mutex);

        delay_loop(SHOPPING_DELAY + (unsigned long)(id % 7) * ARRIVAL_DELAY);
    }

    return NULL;
}

int main(void)
{
    pthread_t stylist;
    pthread_t customer[NUM_CUSTOMERS];
    int i;

    setvbuf(stdout, NULL, _IONBF, 0);

    if (sem_init(&mutex, 0, 1) != 0) {
        die("sem_init mutex");
    }
    if (sem_init(&customers, 0, 0) != 0) {
        die("sem_init customers");
    }
    if (sem_init(&stylist_ready, 0, 0) != 0) {
        die("sem_init stylist_ready");
    }

    if (pthread_create(&stylist, NULL, stylist_thread, NULL) != 0) {
        die("pthread_create stylist");
    }

    delay_loop(CUT_DELAY);

    for (i = 0; i < NUM_CUSTOMERS; i++) {
        if (pthread_create(&customer[i], NULL, customer_thread,
                           (void *)(intptr_t)(i + 1)) != 0) {
            die("pthread_create customer");
        }
        if (i % 4 == 0) {
            delay_loop(ARRIVAL_DELAY);
        }
    }

    for (i = 0; i < NUM_CUSTOMERS; i++) {
        if (pthread_join(customer[i], NULL) != 0) {
            die("pthread_join customer");
        }
    }

    if (pthread_join(stylist, NULL) != 0) {
        die("pthread_join stylist");
    }

    printf("\nSummary\n");
    printf("Given haircuts = %d\n", haircuts_given);
    printf("Salon full = %d times\n", salon_full_score);
    printf("Salon empty = %d times\n", salon_empty_count);
    printf("Shopping trips = %d\n", shopping_trips);

    sem_destroy(&mutex);
    sem_destroy(&customers);
    sem_destroy(&stylist_ready);

    return 0;
}
