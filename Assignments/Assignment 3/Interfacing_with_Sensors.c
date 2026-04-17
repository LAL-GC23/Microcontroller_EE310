/*
 * ---------------------
 * Title:  Interfacing with Sensors
 * ---------------------
 * Purpose:
 * The purpose of this program is to make a safebox system using sensors. 
 * When the system is ON, a light shows the system is working
 * The user enters a 2 digit code using PR1 and PR2
 * Each input shows on the 7 segment display
 * When the code is correct, the motor turns ON
 * When the code is wrong, the buzzer turns ON
 * When the emergency switch is pressed, the buzzer makes a sound
 *
 * Compiler: MPLAB X IDE v6.30
 *
 * Author: Gilberto Cornejo
 *
 * Inputs: PR1 and PR2 (photoresistors), INT0 (emergency switch)
 * Outputs: PORTD (7 segment), RB4 (motor), RB3 (buzzer), RB5 RB6 (LEDs)
 *
 * Date: April 14, 2026
 *
 * Versions:
 *      V1.0: April 11, 2026 - Writing the code 
 *      V1.1: April 15, 2026 - Added interrupt and buzzer
 *
 */

// Initalization 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xc.h> // must have this
//#include "C:/Program Files/Microchip/xc8/v3.10/pic/include/proc/pic18f47k42.h"

#include "Header.h"
#include "Functions.h"
#include "Initialization.h"
#define _XTAL_FREQ 4000000                 // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4

void __interrupt(irq(IRQ_INT0),base(0x4008)) INT0_ISR(void)
{
        // Check if interrupt flag for INT0 is set to 1 - (note INT0 is your input)
    if (PIR1bits.INT0IF == 1 ) {
        while (Buzz_loop < 5){
        PORTBbits.RB3 = 1;
        __delay_ms(250);
        PORTBbits.RB3 = 0;
        __delay_ms(250);
        Buzz_loop++;
        }
    }
            // if so, do something
            // e.g,blink an LED connected to PORTDbits.RD0 for 10 times with a delay of __delay_ms(250)
    
        // always clear the interrupt flag for INT0 when done
    PIR1bits.INT0IF = 0;
        // turn off the led on PORTDbits.RD0 
    
    Buzz_loop = 0;
}

void main(void) {
    
    PORTS_Initialization();
        int Seg_numbers[4] = {0x3F, 0x06, 0x5B, 0x4F};
        // The line above is an array that holds the 7 Seg values displaying 0-3
   
        INTERRUPT_Initialize();
   
        // main code here 
        PORTBbits.RB4 = 0;
        PORTD = Seg_numbers[dig_count];
        
        while(1){ 
        PORTDbits.RD7 = 1; // This is our system light (Decimal Point For 7 Seg)
          
        if (Enter == 0){
            PORTBbits.RB5 = 1;          // Turn ON Input 1 LED
            PORTBbits.RB6 = 0;          // Turn OFF Input 2 LED
            PORTDbits.RD7 = 1; // This is our system light (Decimal Point For 7 Seg)
            if(PORTBbits.RB1 == 1){
                dig_count++;
                PORTD = Seg_numbers[dig_count];
                __delay_ms(1000);
            }
            if(dig_count == 4)
                dig_count = 0;
        }
       
       
       
        // checking if finished entering Num1 and Num2
        if (PORTBbits.RB7 == 1){
            Enter++;
            if(Enter == 1){
                Num1 = dig_count;
                __delay_ms(750);
                dig_count = 0;
                PORTD = Seg_numbers[dig_count];
            }
            if (Enter == 2)
                Num2 = dig_count;
            __delay_ms(750);
        }
       
       
       
        // looking for second input
        if(Enter == 1)
        {
           
            PORTBbits.RB5 = 0;                  // Turn off Input 1 LED
            PORTBbits.RB6 = 1;                  // Turn on Input 2 LED
            PORTDbits.RD7 = 1;                  // This is our system light (Decimal Point For 7 Seg)
            // Check and save second input
            if (PORTBbits.RB2 == 1){
                dig_count++;
                PORTD = Seg_numbers[dig_count];
                
                __delay_ms(1000);
            }
            // Ensures we stay in range and resets to 0 if not
            if (dig_count == 4)
                dig_count = 0;
        }
           
        //after 2 inputs are entered combine to make 1 number 
        
        if (Enter == 2){
            PORTBbits.RB5 = 0;                  // Turn off Input 1 LED
            PORTBbits.RB6 = 0;                  // Turn off Input 2 LED
            PORTDbits.RD7 = 1;                  // This is our system light (Decimal Point For 7 Seg)
            Full_Num = (Num1*16) + (Num2);
           //check if Inputted code is correct
            if (SECRET_CODE == Full_Num){
                PORTBbits.RB4 = 1;
                __delay_ms(3000);
                PORTBbits.RB4 = 0;
                dig_count = 0;
                PORTD = Seg_numbers[dig_count];
                Enter = 0;
                Num1 = 0;
                Num2 =0;
                Full_Num = 0;
            }
           
            //If code is wrong turn on buzzer
            else{
                PORTBbits.RB3 = 1;
                __delay_ms(3000);
                PORTBbits.RB3 = 0;
                dig_count = 0;
                PORTD = Seg_numbers[dig_count];
                Enter = 0;
                Num1 = 0;
                Num2 =0;
                Full_Num = 0;
            }
        }
           
    }
}
