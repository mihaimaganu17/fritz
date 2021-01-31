/* Further details:
 * https://elinux.org/BCM2835_registers#RNG
 * https://github.com/raspberrypi/linux/blob/204050d0eafb565b68abf512710036c10ef1bd23/drivers/char/hw_random/bcm2835-rng.c
 * https://github.com/raspberrypi/linux/blob/rpi-5.4.y/drivers/char/hw_random/iproc-rng200.c
 */

#include "gpio.h"

#define RNG_CTRL        ((volatile unsigned int*)(MMIO_BASE+0x00104000))
#define RNG_STATUS      ((volatile unsigned int*)(MMIO_BASE+0x00104004))
#define RNG_DATA        ((volatile unsigned int*)(MMIO_BASE+0x00104008))
#define RNG_INT_MASK    ((volatile unsigned int*)(MMIO_BASE+0x00104010))

/*
 * Init RNG
 */
void rand_init() {
    *RNG_STATUS = 0x40000;
    // Mask interrupt
    *RNG_INT_MASK |= 1;
    // Enable
    *RNG_CTRL |= 1;
}

/*
 * Return a random number between [min..max]
 */
unsigned int rand(unsigned int min, unsigned int max) {
    // May need to wait for entropy: bits 24-31 store how many words
    // are available for reading; require at least one
    while (!((*RNG_STATUS) >> 24)) {
        asm volatile("nop");
    }
    unsigned int rand_no = *RNG_DATA % (max-min) + min;
    return rand_no;
}
