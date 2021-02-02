/*
 * Framebuffer implementation for Raspberry PI 3
 * Created at 31/01/2021
 * Written by m3m0ry
 */
#include "uart.h"
#include "mbox.h"
#include "homer.h"

unsigned int width, height, pitch, isrgb;   /* Dimensions and channel order */
unsigned *lfb;

/*
 * Set screen resolution to 1024x768
 */
void lfb_init() {
    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SET_PHYS_WH;     // Set physical display width and height
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024;                     // Width
    mbox[6] = 768;                      // Height

    mbox[7] = MBOX_TAG_SET_VIRT_WH;     // Set Virtual Buffer width and height
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024;                    // Width
    mbox[11] = 768;                     // Height

    mbox[12] = MBOX_TAG_SET_VIRT_OFF;   // Set virtual offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;                       // X offset
    mbox[16] = 0;                       // Y offset

    mbox[17] = MBOX_TAG_SET_DEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;

    mbox[21] = MBOX_TAG_SET_PIX_ORD;
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;                       // RGA, no BGR preferably

    mbox[25] = MBOX_TAG_ALLOC_BUFF;
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;
    mbox[29] = 0;

    mbox[30] = MBOX_TAG_GET_PITCH;
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;

    mbox[34] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF;         // Convert GPU address to ARM address
        width = mbox[5];
        height = mbox[6];
        pitch = mbox[33];
        isrgb = mbox[24];
        lfb = (void*)((unsigned long)mbox[28]);
    } else {
        pl011_uart_puts("Unable to set screen resolution to 1024x768x32\n");
    }
}

/*
 * Show a picture
 */
void lfb_showpicture() {
    int x, y;
    unsigned char *ptr = lfb;
    char *data=homer_data, pixel[4];

    ptr += (height - homer_height) / 2*pitch + (width - homer_width) * 2;
    for (y = 0; y < homer_height; y++) {
        for (x = 0; x < homer_width; x++) {
            HEADER_PIXEL(data, pixel);
            *((unsigned int*)ptr)=isrgb ? *((unsigned int *)&pixel) : (unsigned int)(pixel[0]<<16 | pixel[1]<<8 | pixel[2]);
            ptr += 4;
        }
        ptr += pitch - homer_width * 4;
    }
}
