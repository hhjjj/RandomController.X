/* 
 * File:   RandomController.c
 * Author: OSSI
 *
 * Created on 2013년 7월 6일 (토), 오후 5:08
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <uart.h>

/*
 * 
 */

#pragma config CPD = OFF, BOREN = OFF, IESO = OFF, FOSC = INTOSC, FCMEN = OFF, MCLRE = ON, WDTE = OFF, CP = OFF, PWRTE = OFF, CLKOUTEN = OFF,
#pragma config PLLEN = OFF, WRT = OFF, STVREN = OFF, BORV = LO, VCAPEN = OFF, LVP = OFF

void clock_setup(void)
{
    // set internal 4 MHz clock with no PLL
    OSCCON = 0b01101000;
    // just for __delay_ms() as reference
    // this is NOT for setting up clock speed
    #ifndef _XTAL_FREQ
    #define _XTAL_FREQ 4000000
    #endif
}

void port_setup(void)
{
    // output ports
    LATAbits.LATA2 = 0; LATAbits.LATA3 = 0; LATAbits.LATA4 = 0; LATAbits.LATA5 = 0; LATEbits.LATE0 = 0; LATEbits.LATE1 = 0; LATEbits.LATE2 = 0; LATAbits.LATA7 = 0; // 1~8 output
    LATCbits.LATC2 = 0; LATCbits.LATC3 = 0; LATDbits.LATD0 = 0; LATDbits.LATD1 = 0; LATDbits.LATD2 = 0; LATDbits.LATD3 = 0; LATCbits.LATC4 = 0; LATCbits.LATC5 = 0; // 9~16 output

    LATAbits.LATA6 = 0;     // status led

    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;
    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA5 = 0;
    TRISEbits.TRISE0 = 0;
    TRISEbits.TRISE1 = 0;
    TRISEbits.TRISE2 = 0;
    TRISAbits.TRISA7 = 0;

    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISCbits.TRISC4 = 0;
    TRISCbits.TRISC5 = 0;

    TRISAbits.TRISA6 = 0;

    // input ports
    ANSELD = 0; // all pins are digital I/O
    TRISDbits.TRISD4 = 1;   // command in
    TRISDbits.TRISD5 = 1;   // mode select
    TRISDbits.TRISD6 = 1;   // master / slave select
    TRISDbits.TRISD7 = 1;   // test button

    // rotary switch input 0 ~ 5
    OPTION_REGbits.nWPUEN = 0;
    WPUB = 0b00111111;  // all pins are internally pulled-up
    ANSELB = 0; // all pins are digital I/O
    TRISBbits.TRISB0 = 1;   
    TRISBbits.TRISB1 = 1;   
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB3 = 1;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;

    // delay adjustment ADC pin
    TRISAbits.TRISA0 = 1;
    ANSELAbits.ANSA0 = 1;
}



unsigned int adc_read(char chan)
{
    unsigned int val;
    ADCON0bits.CHS = chan;
    ADCON1bits.ADPREF = 0; // VREF+ to VDD
    ADCON1bits.ADNREF = 0; // VREF- to VSS
    ADCON1bits.ADCS = 0b110; // Fosc/64 ~16us at 4MHz Fosc
    ADCON1bits.ADFM = 1; // right justified
    ADCON0bits.ADON = 1; // turn on the ADC module
    ADCON0bits.GO = 1; // start the converstion
    __delay_ms(5);  // some delay..
    val = 0;
    val = (val | ADRESH) << 8;
    val = val | ADRESL;

    return val;
}

unsigned int rotary_switch_read(void)
{
    unsigned int val;
    val = 0;
    val = (~RB2) | ((~RB3) << 1) | ((~RB4) << 2) | ((~RB5) << 3);
    val = val + ((~RB0) | ((~RB1) << 1)) * 10;
    return val;
}

bit mode_read(void)
{
    if(RD5)
    {
        return 1;
    }
    else  if (RD5 == 0)
    {
        return 0;
    }
}

bit master_select_read(void)
{
    if(RD6)
    {
        return 1;
    }
    else  if (RD6 == 0)
    {
        return 0;
    }
}

bit test_button_read(void)
{
    if(RD7)
    {
        return 1;
    }
    else  if (RD7 == 0)
    {
        return 0;
    }
}

bit cmd_in_read(void)
{
    if(RD4)
    {
        return 1;
    }
    else  if (RD4 == 0)
    {
        return 0;
    }
}



volatile unsigned int delay_adj_val;
volatile unsigned int rotary_switch_val;

void main(void) {
    
    port_setup();
    clock_setup();
    uart_setup();
    delay_adj_val = 0;
    while(1)
    {
        LATA6= 1;
//        LATAbits.LATA2 = 1; LATAbits.LATA3 = 1; LATAbits.LATA4 = 1; LATAbits.LATA5 = 1; LATEbits.LATE0 = 1; LATEbits.LATE1 = 1; LATEbits.LATE2 = 1; LATAbits.LATA7 = 1; // 1~8 output
//        LATCbits.LATC2 = 1; LATCbits.LATC3 = 1; LATDbits.LATD0 = 1; LATDbits.LATD1 = 1; LATDbits.LATD2 = 1; LATDbits.LATD3 = 1; LATCbits.LATC4 = 1; LATCbits.LATC5 = 1; // 9~16 output
        LATAbits.LATA2 = 1;
        __delay_ms(500);

        delay_adj_val = adc_read(0);
        rotary_switch_val = rotary_switch_read();
        printf("Dealy Adj Value %d\n",delay_adj_val);
        printf("Rotary Val %d\n",rotary_switch_val);

        if (mode_read())
        {
            printf("MODE 1\n");
        }
        else
        {
             printf("MODE 2\n");
        }
        
        if (master_select_read())
        {
            printf("Master \n");
        }
        else
        {
             printf("Slave \n");
        }
        
        if (test_button_read())
        {
            printf("test button NOT pushed\n");
        }
        else
        {
             printf("test button pushed\n");
        }

        if (cmd_in_read())
        {
            printf("NO command\n");
        }
        else
        {
             printf("Command In!!\n");
        }

        LATA6= 0;


        LATAbits.LATA2 = 0;

        __delay_ms(500);
    
    }
}

