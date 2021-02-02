#pragma once
/*
 * Mbox is an interface for raspberryPI use for interprocess communication
 * (between ARMCPU and VideoCore GPU
 * More info: https://jsandler18.github.io/extra/mailbox.html
 * Created at 27/01/2021
 * Written by m3m0ry
 */

/* a properly aligned buffer */
extern volatile unsigned int mbox[36];

#define MBOX_REQUEST    0

/* Channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags 
 * https://www.valvers.com/open-software/raspberry-pi/bare-metal-programming-in-c-part-5/#mailboxes
 * https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/image-processing/
 * Have the following form 0x000XYZZZ, where:
 * - X = which hardware device is accessed 
 * - Y = which type of commmand it is (0=get, 4=test, 8=set)
 * - ZZZ is the specific command */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_SETPOWER       0x28001
#define MBOX_TAG_LAST           0
#define MBOX_TAG_CLKRATE        0x38002

#define MBOX_TAG_SET_PHYS_WH    0x48003
#define MBOX_TAG_SET_VIRT_WH    0x48004
#define MBOX_TAG_SET_VIRT_OFF   0x48009
#define MBOX_TAG_SET_DEPTH      0x48005
#define MBOX_TAG_SET_PIX_ORD    0x48006
#define MBOX_TAG_ALLOC_BUFF     0x40001
#define MBOX_TAG_GET_PITCH      0x40008

int mbox_call(unsigned char ch);
