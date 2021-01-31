/*
 * Functions responsible for waiting certain amounts of time
 * 3 ways:
 * - 1st: CPU frequency dependent(useful if time is given in CPU clock cycles)
 * - 2nd and 3rd: Microsec(millionth of a second) based.
 * Created at 31/01/2021
 * Written by m3mory
 */

void wait_cycles(unsigned int n);
void wait_msec(unsigned int n);
unsigned long get_system_timer();
void wait_msec_st(unsigned int n);
