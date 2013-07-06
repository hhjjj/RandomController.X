#include <uart.h>

/*
 * Baud rate = 9600
 * at Clock = 4 MHz
 */
void uart_setup(void)
{
    // set direction for UART pins
    TRISCbits.TRISC6 = 0;   // output for TX pin
    TRISCbits.TRISC7 = 1;   // input for RX pin

    // set baud rate to 9600 bps
    BRGH = 1;
    BRG16 = 1;
    SPBRGH = 0;
    SPBRGL = 103;

    // enabling TX and RX in Async Mode
    TXEN = 1;
    CREN = 1;
    SYNC = 0;
    SPEN = 1;

}

// putch has to be implemented for printf() in stdio.h
void putch(char b)
{
    while (!TRMT);
    TXREG = b;
}

