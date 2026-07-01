#include "monitor.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CUT_DELAY 2500000UL
#define SHOPPING_DELAY 700000UL
#define ARRIVAL_DELAY 50000UL

static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

static void delay_loop(unsigned long bound)
{
    volatile unsigned long i;

    for (i = 0; i < bound; i++) {
    }
}

static void *stylist_thread(void *arg)
{
    (void)arg;

    while (!mon_allDone()) {
        mon_debugPrint();
        mon_checkCustomer();
        delay_loop(CUT_DELAY);
        if (mon_finishHaircut()) {
            break;
        }
    }

    return NULL;
}

static void *customer_thread(void *arg)
{
    int id = (int)(intptr_t)arg;

    delay_loop((unsigned long)(id % 11) * ARRIVAL_DELAY);

    while (1) {
        mon_debugPrint();
        if (mon_checkStylist(id)) {
            break;
        }
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
    mon_init();

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

    mon_destroy();
    return 0;
}
