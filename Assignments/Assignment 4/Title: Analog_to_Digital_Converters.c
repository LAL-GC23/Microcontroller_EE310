/*---------------------
 Title: Analog_to_Digital_Converters
---------------------
 Purpose:
 The purpose of this program is to read analog data from an accelerometer using the ADC 
 and display the motion state on an LCD screen.
 The program continuously reads voltage from the sensor and converts it to acceleration X-dir
 Based on the value, it determines the state of the system:
 If the voltage changes quickly it displays Shake
 If tilted left it displays Tilt Left
 If tilted right it displays Tilt Right
 If no movement it displays Flat
 The current acceleration value is also shown on the LCD.
 And when an interrupt is triggered on RC2, an LED on RD3 will blink multiple times.

 Compiler: MPLAB X IDE v6.30

 Author: Gilberto Cornejo

 Inputs:
 RA0 is the analog input from the accelerometer
 RC2 is the interrupt input (button or signal trigger)

 Outputs:
 LCD display uses PORTB and PORTD control pins
 RD3 is the LED output that blinks during interrupt

 Date: April 3 2026

 Versions:
       V1.0: Initial code with ADC reading and LCD display
       V1.1: Added motion detection (tilt, shake, flat)
       V1.2: Added interrupt functionality with LED blinking

---------------------*/

#pragma config FEXTOSC = LP
#pragma config RSTOSC = EXTOSC
#pragma config CLKOUTEN = OFF
#pragma config PR1WAY = ON
#pragma config CSWEN = ON
#pragma config FCMEN = ON
#pragma config MCLRE = EXTMCLR
#pragma config PWRTS = PWRT_OFF
#pragma config MVECEN = ON
#pragma config IVT1WAY = ON
#pragma config LPBOREN = OFF
#pragma config BOREN = SBORDIS
#pragma config BORV = VBOR_2P45
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON
#pragma config DEBUG = OFF
#pragma config XINST = OFF
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC
#pragma config BBSIZE = BBSIZE_512
#pragma config BBEN = OFF
#pragma config SAFEN = OFF
#pragma config WRTAPP = OFF
#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config WRTSAF = OFF
#pragma config LVP = ON
#pragma config CP = OFF

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define _XTAL_FREQ 4000000
#define FCY _XTAL_FREQ/4

#define RS LATD0
#define EN LATD1
#define ldata LATB

#define LCD_Port TRISB
#define LCD_Control TRISD

#define Vref 3.3

void LCD_Init();
void LCD_Command(char);
void LCD_Char(char x);
void LCD_String(const char *);
void LCD_String_xy(char, char, const char *);
void MSdelay(unsigned int);
void INTERRUPT_Initialize();
void ADC_Init();
void LCD_Clear();

int light_loop = 0;

int digital;
float voltage;
float accel_x;
float prev_voltage = 0;

char data[16];
char state[17];

void main(void)
{
    IOCCFbits.IOCCF2 = 0;
    PIR0bits.IOCIF = 0;

    INTERRUPT_Initialize();

    IOCCFbits.IOCCF2 = 0;
    PIR0bits.IOCIF = 0;

    ADC_Init();
    LCD_Init();

    IOCCFbits.IOCCF2 = 0;

    while(1)
    {
        ADCON0bits.GO = 1;

        while(ADCON0bits.GO)
        {
            // Wait until ADC conversion is done
        }

        digital = (ADRESH * 256) | ADRESL;

        voltage = digital * ((float)Vref / (float)4096);

        accel_x = (voltage - 1.06) / 0.33;

if(fabs(voltage - prev_voltage) > 0.10)
{
    strcpy(state, "Shake");
}
else if(accel_x < -0.20)
{
    strcpy(state, "Tilt Left");
}
else if(accel_x > 0.20)
{
    strcpy(state, "Tilt Right");
}
else
{
    strcpy(state, "Flat");
}

prev_voltage = voltage;

        sprintf(data, "%.2f X-dir", accel_x);

        LCD_Command(0x01);
        LCD_String_xy(1, 0, "State:");
        LCD_String_xy(1, 7, state);
        LCD_String_xy(2, 0, data);

        __delay_ms(300);
    }
}

void __interrupt(irq(default), base(0x6008)) ISR(void)
{
    if(IOCCFbits.IOCCF2 == 1)
    {
        while(light_loop < 10)
        {
            PORTDbits.RD3 = 0;
            __delay_ms(500);

            PORTDbits.RD3 = 1;
            __delay_ms(500);

            light_loop++;
        }

        light_loop = 0;
        PORTDbits.RD3 = 0;

        IOCCFbits.IOCCF2 = 0;
        PIR0bits.IOCIF = 0;
    }
}

void INTERRUPT_Initialize(void)
{
    INTCON0bits.IPEN = 1;
    INTCON0bits.GIEH = 1;
    INTCON0bits.GIEL = 1;
    INTCON0bits.GIE = 1;

    TRISCbits.TRISC2 = 1;
    ANSELCbits.ANSELC2 = 0;

    TRISDbits.TRISD3 = 0;
    ANSELDbits.ANSELD3 = 0;

    IOCCPbits.IOCCP2 = 1;

    IPR0bits.IOCIP = 1;
    PIE0bits.IOCIE = 1;

    PIR0bits.IOCIF = 0;
    IOCCFbits.IOCCF2 = 0;

    IVTBASEU = 0x00;
    IVTBASEH = 0x60;
    IVTBASEL = 0x08;
}

void ADC_Init(void)
{
    ADCON0bits.FM = 1;
    ADCON0bits.CS = 1;

    TRISAbits.TRISA0 = 1;
    ANSELAbits.ANSELA0 = 1;

    ADPCH = 0x00;

    ADCLK = 0x00;

    ADRESH = 0;
    ADRESL = 0;

    ADPREL = 0;
    ADPREH = 0;

    ADACQL = 0;
    ADACQH = 0;

    ADREF = 0x00;

    ADCON0bits.ON = 1;
}

void LCD_Init()
{
    MSdelay(15);

    LCD_Port = 0x00;
    LCD_Control = 0x00;

    LATC = 0b00000000;
    ANSELC = 0b00000000;
    PORTC = 0b00000000;
    TRISC = 0b00000100;

    LATA = 0b00000000;
    ANSELA = 0b00000001;
    PORTA = 0b00000000;
    TRISA = 0b00000001;

    LCD_Command(0x01);
    LCD_Command(0x38);
    LCD_Command(0x0c);
    LCD_Command(0x06);
}

void LCD_Clear()
{
    LCD_Command(0x01);
}

void LCD_Command(char cmd)
{
    ldata = cmd;
    RS = 0;
    EN = 1;
    NOP();
    EN = 0;
    MSdelay(3);
}

void LCD_Char(char dat)
{
    ldata = dat;
    RS = 1;
    EN = 1;
    NOP();
    EN = 0;
    MSdelay(1);
}

void LCD_String(const char *msg)
{
    while((*msg) != 0)
    {
        LCD_Char(*msg);
        msg++;
    }
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char location = 0;

    if(row <= 1)
    {
        location = (0x80) | ((pos) & 0x0f);
        LCD_Command(location);
    }
    else
    {
        location = (0xC0) | ((pos) & 0x0f);
        LCD_Command(location);
    }

    LCD_String(msg);
}

void MSdelay(unsigned int val)
{
    unsigned int i, j;

    for(i = 0; i < val; i++)
    {
        for(j = 0; j < 165; j++);
    }
}
