#ifndef MONITOR_H
#define MONITOR_H

#define NUM_CUSTOMERS 120
#define WAITING_CHAIRS 7

void mon_init(void);
void mon_destroy(void);
void mon_debugPrint(void);
void mon_checkCustomer(void);
int mon_checkStylist(int customer_id);
int mon_finishHaircut(void);
int mon_allDone(void);

#endif
