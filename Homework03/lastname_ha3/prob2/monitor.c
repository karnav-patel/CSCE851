#include "monitor.h"

#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct cond {
    int blocked;
    sem_t queue;
    const char *name;
} CV;

static sem_t entry_queue;
static sem_t urgent_queue;
static int urgent_count = 0;

static CV stylistAvailable;
static CV customerAvailable;

static int customers_waiting = 0;
static int stylist_tokens = 0;
static int haircuts_given = 0;
static int salon_full_score = 0;
static int salon_empty_count = 0;

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

static void cv_init(CV *cv, const char *name)
{
    cv->blocked = 0;
    cv->name = name;
    if (sem_init(&cv->queue, 0, 0) != 0) {
        die("sem_init cv");
    }
}

static int cv_count(CV *cv)
{
    return cv->blocked;
}

static void monitor_enter(void)
{
    sem_down(&entry_queue);
}

static void monitor_exit(void)
{
    if (urgent_count > 0) {
        sem_up(&urgent_queue);
    } else {
        sem_up(&entry_queue);
    }
}

static void cv_wait(CV *cv)
{
    cv->blocked++;
    printf("[monitor] wait(%s): releasing monitor and blocking; blocked=%d\n",
           cv->name, cv->blocked);

    if (urgent_count > 0) {
        sem_up(&urgent_queue);
    } else {
        sem_up(&entry_queue);
    }

    sem_down(&cv->queue);
    cv->blocked--;

    printf("[monitor] wait(%s): resumed at the statement after wait; blocked=%d\n",
           cv->name, cv->blocked);
}

static void cv_signal(CV *cv)
{
    if (cv->blocked > 0) {
        urgent_count++;
        printf("[monitor] signal(%s): signaler yields to awakened thread "
               "(signal-and-wait).\n", cv->name);
        sem_up(&cv->queue);
        sem_down(&urgent_queue);
        urgent_count--;
        printf("[monitor] signal(%s): signaler resumes after awakened thread "
               "left or waited again.\n", cv->name);
    }
}

static void print_state_locked(void)
{
    int i;

    printf("|");
    for (i = 0; i < WAITING_CHAIRS; i++) {
        printf(" %d |", i < customers_waiting ? 1 : 0);
    }
    printf(" => %d\n", customers_waiting);
    printf("Given haircuts = %d\n", haircuts_given);
    printf("Salon full = %d times\n", salon_full_score);
    printf("Salon empty = %d times\n", salon_empty_count);
    printf("CV counts: stylistAvailable=%d, customerAvailable=%d, urgent=%d\n\n",
           cv_count(&stylistAvailable), cv_count(&customerAvailable),
           urgent_count);
}

void mon_init(void)
{
    if (sem_init(&entry_queue, 0, 1) != 0) {
        die("sem_init entry_queue");
    }
    if (sem_init(&urgent_queue, 0, 0) != 0) {
        die("sem_init urgent_queue");
    }

    cv_init(&stylistAvailable, "stylistAvailable");
    cv_init(&customerAvailable, "customerAvailable");
}

void mon_destroy(void)
{
    sem_destroy(&entry_queue);
    sem_destroy(&urgent_queue);
    sem_destroy(&stylistAvailable.queue);
    sem_destroy(&customerAvailable.queue);
}

void mon_debugPrint(void)
{
    monitor_enter();
    print_state_locked();
    monitor_exit();
}

void mon_checkCustomer(void)
{
    monitor_enter();

    stylist_tokens++;
    printf("[stylist] is ready and signals stylistAvailable.\n");
    cv_signal(&stylistAvailable);

    if (customers_waiting == 0) {
        salon_empty_count++;
        printf("[stylist] salon is empty; stylist sleeps on customerAvailable.\n");
        print_state_locked();
        cv_wait(&customerAvailable);
        printf("[stylist] awakened by a customer and continues after wait.\n");
    }

    customers_waiting--;
    printf("[stylist] takes one customer from the waiting chairs.\n");
    print_state_locked();

    monitor_exit();
}

int mon_checkStylist(int customer_id)
{
    int accepted = 0;

    monitor_enter();

    if (customers_waiting < WAITING_CHAIRS) {
        customers_waiting++;

        if (customers_waiting == WAITING_CHAIRS) {
            salon_full_score += WAITING_CHAIRS;
            printf("[customer %03d] takes the seventh waiting chair; salon is full.\n",
                   customer_id);
        } else {
            printf("[customer %03d] sits in a waiting chair.\n", customer_id);
        }
        print_state_locked();

        cv_signal(&customerAvailable);

        if (stylist_tokens == 0) {
            printf("[customer %03d] stylist is not available; waiting on "
                   "stylistAvailable.\n", customer_id);
            cv_wait(&stylistAvailable);
            printf("[customer %03d] resumes immediately after "
                   "wait(stylistAvailable).\n", customer_id);
        }

        stylist_tokens--;
        printf("[customer %03d] moves to the styling chair.\n\n", customer_id);
        accepted = 1;
    } else {
        printf("[customer %03d] finds all chairs full and goes shopping.\n",
               customer_id);
        print_state_locked();
    }

    monitor_exit();
    return accepted;
}

int mon_finishHaircut(void)
{
    int done;

    monitor_enter();
    haircuts_given++;
    done = haircuts_given >= NUM_CUSTOMERS;
    printf("[stylist] finished haircut %d of %d.\n\n",
           haircuts_given, NUM_CUSTOMERS);
    if (done) {
        printf("[stylist] all customers have received haircuts; shop closes.\n\n");
    }
    monitor_exit();

    return done;
}

int mon_allDone(void)
{
    int done;

    monitor_enter();
    done = haircuts_given >= NUM_CUSTOMERS;
    monitor_exit();

    return done;
}
