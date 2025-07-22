#include "stdint.h"
#include "include/util.h"
#include "include/idt.h"
#include "include/kernel.h"
#include "include/VGA.h"
#include "include/timer.h"

uint64_t ticks;
const uint32_t freq = 10;

void onIRQ0(struct InterruptRegisters *regs) {
    ticks += 1;
    printf("timer ticked");

}

void init_Timer() {
    ticks = 0;
    irq_install_handler(0, &onIRQ0);

    uint32_t divisor = 1193180/freq;
    //0011 0110
    outPortB(0x43, 0x36);
    outPortB(0x40, (uint8_t)(divisor & 0xFF));
    outPortB(0x40, (uint8_t)(divisor >> 8) & 0xFF);
}