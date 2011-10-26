#ifndef MM_CLOCK_H
#define MM_CLOCK_H

/* Routines for using cycle counter */

/* Start the counter */
void start_counter(void);

/* Get # cycles since counter started */
double get_counter(void);

/* Measure overhead for counter */
double ovhd(void);

/* Determine clock rate of processor (using a default sleeptime) */
double mhz(int verbose);

/* Determine clock rate of processor, having more control over accuracy */
double mhz_full(int verbose, int sleeptime);

/** Special counters that compensate for timer interrupt overhead */

void start_comp_counter(void);

double get_comp_counter(void);

#endif /* MM_CLOCK_H */
