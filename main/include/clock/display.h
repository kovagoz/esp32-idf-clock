#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#define DISPLAY_MSG_BOOT 0x7c,0x5c,0x5c,0x78
#define DISPLAY_MSG_SYNC 0x6d,0x66,0x54,0x39
#define DISPLAY_MSG_FAIL 0x71,0x77,0x06,0x38
#define DISPLAY_MSG_FINE 0x71,0x06,0x54,0x79

void display_init();
void display_write(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3);
void display_write_time(int hours, int minutes);

#endif
