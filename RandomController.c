/* 
 * File:   RandomController.c
 * Author: OSSI
 *
 * Created on 2013년 7월 6일 (토), 오후 5:08
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <RandomController.h>
#include <uart.h>


/*
 * 
 */

#pragma config CPD = OFF, BOREN = OFF, IESO = OFF, FOSC = INTOSC, FCMEN = OFF, MCLRE = ON, WDTE = OFF, CP = OFF, PWRTE = OFF, CLKOUTEN = OFF,
#pragma config PLLEN = OFF, WRT = OFF, STVREN = OFF, BORV = LO, VCAPEN = OFF, LVP = OFF

#define PIEZO_TOTAL 16

unsigned int PlayOrder[PIEZO_TOTAL]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
volatile unsigned int PlayCount;
volatile unsigned int delay_adj_val;
volatile unsigned int rotary_switch_val;
volatile unsigned int counter;

volatile unsigned int cmd_count;

volatile unsigned int MasterFlag;


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
    // output ports value set to 0
    // piezo ports
    LATAbits.LATA2 = 0; LATAbits.LATA3 = 0; LATAbits.LATA4 = 0; LATAbits.LATA5 = 0; LATEbits.LATE0 = 0; LATEbits.LATE1 = 0; LATEbits.LATE2 = 0; LATAbits.LATA7 = 0; // 1~8 output
    LATCbits.LATC2 = 0; LATCbits.LATC3 = 0; LATDbits.LATD0 = 0; LATDbits.LATD1 = 0; LATDbits.LATD2 = 0; LATDbits.LATD3 = 0; LATCbits.LATC4 = 0; LATCbits.LATC5 = 0; // 9~16 output

    // status led port
    LATAbits.LATA6 = 0;     

    // ports directions to outputs
    // piezo ports
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

    // status led port
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
    if(val > 16)
    {
        val = 16;
    }
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

void piezo_on (char chan)
{
    switch(chan)
    {
        case 0:
            break;
        case 1:
            LATAbits.LATA2 = 1;
            break;
        case 2:
            LATAbits.LATA3 = 1;
            break;
        case 3:
            LATAbits.LATA4 = 1;
            break;
        case 4:
            LATAbits.LATA5 = 1;
            break;
        case 5:
            LATEbits.LATE0 = 1;
            break;
        case 6:
            LATEbits.LATE1 = 1;
            break;
        case 7:
            LATEbits.LATE2 = 1;
            break;
        case 8:
            LATAbits.LATA7 = 1;
            break;
        case 9:
            LATCbits.LATC2 = 1;
            break;
        case 10:
            LATCbits.LATC3 = 1;
            break;
        case 11:
            LATDbits.LATD0 = 1;
            break;
        case 12:
            LATDbits.LATD1 = 1;
            break;
        case 13:
            LATDbits.LATD2 = 1;
            break;
        case 14:
            LATDbits.LATD3 = 1;
            break;
        case 15:
            LATCbits.LATC4 = 1;
            break;
        case 16:
            LATCbits.LATC5 = 1;
            break;
        default:
            break;
    }

}

void piezo_off(char chan)
{
    switch(chan)
    {
        case 0:
            break;
        case 1:
            LATAbits.LATA2 = 0;
            break;
        case 2:
            LATAbits.LATA3 = 0;
            break;
        case 3:
            LATAbits.LATA4 = 0;
            break;
        case 4:
            LATAbits.LATA5 = 0;
            break;
        case 5:
            LATEbits.LATE0 = 0;
            break;
        case 6:
            LATEbits.LATE1 = 0;
            break;
        case 7:
            LATEbits.LATE2 = 0;
            break;
        case 8:
            LATAbits.LATA7 = 0;
            break;
        case 9:
            LATCbits.LATC2 = 0;
            break;
        case 10:
            LATCbits.LATC3 = 0;
            break;
        case 11:
            LATDbits.LATD0 = 0;
            break;
        case 12:
            LATDbits.LATD1 = 0;
            break;
        case 13:
            LATDbits.LATD2 = 0;
            break;
        case 14:
            LATDbits.LATD3 = 0;
            break;
        case 15:
            LATCbits.LATC4 = 0;
            break;
        case 16:
            LATCbits.LATC5 = 0;
            break;
        default:
            break;
    }
}

void piezo_all_on(void)
{
    // turns on all piezo channed set by rotary switch
    unsigned int val;
    volatile int i;
    val = rotary_switch_read();

    for (i = 1; i < val+1; i++)
    {
        piezo_on(i);
    }
}

void piezo_all_off(void)
{
    // turns all off
    LATAbits.LATA2 = 0; LATAbits.LATA3 = 0; LATAbits.LATA4 = 0; LATAbits.LATA5 = 0; LATEbits.LATE0 = 0; LATEbits.LATE1 = 0; LATEbits.LATE2 = 0; LATAbits.LATA7 = 0; // 1~8 output
    LATCbits.LATC2 = 0; LATCbits.LATC3 = 0; LATDbits.LATD0 = 0; LATDbits.LATD1 = 0; LATDbits.LATD2 = 0; LATDbits.LATD3 = 0; LATCbits.LATC4 = 0; LATCbits.LATC5 = 0; // 9~16 output
}


unsigned int random_number_generator(unsigned int seed, unsigned int max)
{
    srand(seed);
    return (rand() % max);
}

void piezo_random_on(char order)
{

        // order range: 0 ~ 15
        // Status LED Off
        LATAbits.LATA6= 0;

        piezo_on(PlayOrder[order]);
        printf("PlayCount: %d", PlayCount);

        // piezo on delay
        __delay_ms(300);

        piezo_all_off();

        // Status LED On
        LATAbits.LATA6= 1;
}

void shuffle(unsigned int *array, int size)
{
    // initialize the order
    for (int i = 0; i < PIEZO_TOTAL ; i++)
    {
        PlayOrder[i] = i+1;
    }

    for (int a = size -1; a > 0 ; a--)
    {
        int r = rand() % (a+1);
        if ( r != a)
        {
            int temp = array[a];
            array[a] = array[r];
            array[r] = temp;
        }
    }
}

void printArray(unsigned int *array, int size)
{
    printf("\r\n");
    for(int a = 0 ; a < size; a++)
    {

        printf("%dth: %d\r\n",a, array[a]);

    }
    printf("\r\n");
}

void main(void) {
    
    port_setup();
    clock_setup();
    uart_setup();
    delay_adj_val = 0;
    counter = 0;

    // Wait for init
    __delay_ms(100);
    rotary_switch_val = rotary_switch_read();
    printf("Rotary Val %d\n",rotary_switch_val);
    shuffle(PlayOrder,rotary_switch_val);


    PlayCount = 0;

    cmd_count = 0;

    MasterFlag = 0;

     // Status LED On
     LATAbits.LATA6= 1;

    while(1)
    {

        
//        // Status LED On
//        LATAbits.LATA6= 1;
//
//
//
//
//        // piezo on time = 100 ms
//        __delay_ms(100);

       
        // Check
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
            MasterFlag = 1;
        }
        else
        {
             printf("Slave \n");
             MasterFlag = 0;
        }
        
        if (test_button_read())
        {
            
            printf("test button NOT pushed\n");
        }
        else
        {
            printf("test button pushed\n");
            rotary_switch_val = rotary_switch_read();
            printf("Rotary Val %d\n",rotary_switch_val);
            piezo_all_on();
            shuffle(PlayOrder,rotary_switch_val);
            printArray(PlayOrder,rotary_switch_val);
             
        }

        if (cmd_in_read())
        {

            //printf("NO command\n");
        }
        else
        {
            if (MasterFlag == 0)
            {
                // slave block begin
                cmd_count++;
                if ( cmd_count > 1)
                {
                    piezo_random_on(PlayCount);
                    PlayCount++;
                    if ( PlayCount > 15)
                    {
                        PlayCount = 0;
                    }




                    cmd_count = 0;
                    // piezo_random_on
                }
                 //printf("Command In!!\n");
                // slave block end
            }
        }

//        // Status LED Off
//        LATAbits.LATA6= 0;
//
//
//        piezo_all_off();

        
//        /* Piezo On Block Begin */
//        // Status LED Off
//        LATAbits.LATA6= 0;
//
//        piezo_on(PlayOrder[PlayCount]);
//        printf("PlayCount: %d", PlayCount);
//        PlayCount++;
//        if ( PlayCount > 15)
//        {
//            PlayCount = 0;
//            // Shuffle Again for New Random Order
//            rotary_switch_val = rotary_switch_read();
//            printf("Rotary Val %d\n",rotary_switch_val);
//            shuffle(PlayOrder,rotary_switch_val);
//        }
//
//        // piezo on delay
//        __delay_ms(300);
//
//        piezo_all_off();
//
//        // Status LED On
//        LATAbits.LATA6= 1;
//        /* Piezo On Block End */


        // Master Mode
        if(MasterFlag)
        {
            piezo_random_on(0);
            __delay_ms(400);
            piezo_random_on(1);
            __delay_ms(100);
            piezo_random_on(2);
            __delay_ms(160);
            piezo_random_on(3);
            __delay_ms(900);

            piezo_random_on(4);
            __delay_ms(90);
            piezo_random_on(5);
            __delay_ms(160);
            piezo_random_on(6);
            __delay_ms(100);

            shuffle(PlayOrder,rotary_switch_val);

            piezo_random_on(0);
            __delay_ms(600);

            piezo_random_on(1);
            __delay_ms(500);

            piezo_random_on(2);
            __delay_ms(1000);
            piezo_random_on(3);
            __delay_ms(150);
            piezo_random_on(4);
            __delay_ms(500);

            piezo_random_on(5);
            __delay_ms(100);
            piezo_random_on(6);
            __delay_ms(300);
            piezo_random_on(1);
            __delay_ms(1200);
            piezo_random_on(5);
            __delay_ms(600);

            rotary_switch_val = rotary_switch_read();
            printf("Rotary Val %d\n",rotary_switch_val);
            random_number_generator((unsigned int)counter,200);
            shuffle(PlayOrder,rotary_switch_val);
        }
        


//        /* Random Delay Block Begin */
//        delay_adj_val = adc_read(0);
//        printf("Delay Adj Value %d\n",delay_adj_val);
//        unsigned long delay_long = 0;
//        delay_long = delay_adj_val * 16;
//        printf("Delay Long %u\n",delay_long);
//        unsigned int sec = 0;
//        sec = (unsigned int)(delay_long / 1000);
//
//        printf("random number: %u\n",random_number_generator((unsigned int)counter,100));
//
//        sec = sec * 1000 + 1000 + random_number_generator((unsigned int)counter,200) * 10 ;
//
//        volatile unsigned int i;
//        for (i = 0 ; i < sec ; i++)
//        {
//        __delay_ms(1);
//        }
//
//        /* Random Delay Block End */



        /* Seed Number Block Begin */
        counter++;
        if( counter > 60000)
        {
            counter = 0;
        }
        /* Seed Number Block End */
    }
}

