//-----------------------------
// Title: Heating and Cooling Control System
//-----------------------------
// Purpose: This program compares a measured temperature with a reference 
// temperature and turns on heating, cooling, or nothing based on the result.
//
// Dependencies: myConfigFile.inc and xc.inc
//
// Compiler: MPLAB X IDE v6.30 (Simulator)
//
// Author: Gilberto Cornejo
//
// OUTPUTS: 
// PORTD.2 controls the cooling system LED.
// PORTD.1 controls the heating system LED.
// contREG (0x22) stores the control value for the system.
// Registers 0x60-0x62 store the decimal digits of the reference temperature.
// Registers 0x70-0x72 store the decimal digits of the measured temperature.
//
// INPUTS: 
// measuredTempInput is the measured temperature value from the sensor.
// refTempInput is the reference temperature value entered by the user.
//
// Versions:
// V1.0: March 05, 2026 - Initial version of the program
// V1.1: March 12, 2026 - Updated comparison logic and decimal conversion
//-----------------------------

;----------------------------------
; Setup files
;----------------------------------
#include "C:\Users\gilberto\MPLABXProjects\FirstAssemblyPrograming.X\myConfigFile.inc"
#include <xc.inc>

;----------------
; PROGRAM INPUTS
;----------------
#define measuredTempInput  -5
#define refTempInput       15

;---------------------
; Definitions
;---------------------
#define SWITCH   LATD,2
#define LED0     PORTD,0
#define LED1     PORTD,1

;---------------------
; Program Constants
;---------------------
REG10   EQU 10h
REG11   EQU 11h
REG01   EQU 1h

;----------------------------------
; Main registers
;----------------------------------
refTempREG               EQU 0x20
measuredTempREG          EQU 0x21
contREG                  EQU 0x22

;----------------------------------
; Work registers
;----------------------------------
NUMERATOR                EQU 0x30
NUMERATOR2               EQU 0x31
QUOTIENT                 EQU 0x40
QUOTIENT2                EQU 0x41
TEN                      EQU 10

;----------------------------------
; Decimal digit registers
;----------------------------------
refTempDecimalLOW        EQU 0x60   ; ones digit
refTempDecimalMID        EQU 0x61   ; tens digit
refTempDecimalHIGH       EQU 0x62   ; hundreds digit

measuredTempDecimalLOW   EQU 0x70   ; ones digit
measuredTempDecimalMID   EQU 0x71   ; tens digit
measuredTempDecimalHIGH  EQU 0x72   ; hundreds digit

;----------------------------------
; MAIN PROGRAM
;----------------------------------
PSECT absdata,abs,ovrld

    ORG 0x20

;----------------------------------
; Clear PORTD and make it output
;----------------------------------
    CLRF    PORTD
    MOVLW   0x00
    MOVWF   TRISD, A

;----------------------------------
; Put input values in registers
;----------------------------------
    MOVLW   measuredTempInput
    MOVWF   measuredTempREG

    MOVLW   refTempInput
    MOVWF   refTempREG

    CLRF    contREG

;----------------------------------
; Check ref temp is from 10 to 50
; This allows 10 and 50 too
;----------------------------------
_refcheck:
    MOVLW   0x0A
    CPFSEQ  refTempREG
    GOTO    _checkRefLow
    GOTO    _checkRefHigh

_checkRefLow:
    MOVLW   0x0A
    CPFSGT  refTempREG
    SLEEP

_checkRefHigh:
    MOVLW   0x32
    CPFSEQ  refTempREG
    GOTO    _checkRefTop
    GOTO    _signcheck

_checkRefTop:
    MOVLW   0x32
    CPFSLT  refTempREG
    SLEEP

    GOTO    _signcheck

;----------------------------------
; If measured temp is negative,
; go to heat right away
;----------------------------------
_signcheck:
    BTFSC   measuredTempREG, 7
    GOTO    _HEAT
    GOTO    _equal

;----------------------------------
; Check if temps are equal
;----------------------------------
_equal:
    MOVF    refTempREG, 0
    CPFSEQ  measuredTempREG
    GOTO    _comparison
    GOTO    _NOTHING

;----------------------------------
; If not equal, choose heat or cool
;----------------------------------
_comparison:
    MOVF    refTempREG, 0
    CPFSGT  measuredTempREG
    GOTO    _HEAT
    GOTO    _COOL

;----------------------------------
; Too hot
; contREG = 2
; turn on PORTD.2
;----------------------------------
_COOL:
    MOVLW   0x02
    MOVWF   contREG

    MOVLW   0b00000100
    MOVWF   PORTD, A

    GOTO    _measuredDECIMAL

;----------------------------------
; Too cold
; contREG = 1
; turn on PORTD.1
;----------------------------------
_HEAT:
    MOVLW   0x01
    MOVWF   contREG

    MOVLW   0b00000010
    MOVWF   PORTD, A

    GOTO    _measuredDECIMAL

;----------------------------------
; Temps are same
; contREG = 0
; turn off output
;----------------------------------
_NOTHING:
    MOVLW   0x00
    MOVWF   contREG

    MOVLW   0b00000000
    MOVWF   PORTD, A

    GOTO    _measuredDECIMAL

;----------------------------------
; Split measured temp into digits
;----------------------------------
_measuredDECIMAL:
    MOVF    measuredTempREG, 0
    MOVWF   NUMERATOR

    MOVLW   TEN
    CLRF    QUOTIENT

_D1:
    INCF    QUOTIENT
    SUBWF   NUMERATOR
    BC      _D1

    ADDWF   NUMERATOR
    DECF    QUOTIENT

    MOVFF   NUMERATOR, measuredTempDecimalLOW
    MOVFF   QUOTIENT, NUMERATOR
    CLRF    QUOTIENT

_D2:
    INCF    QUOTIENT
    SUBWF   NUMERATOR
    BC      _D2

    ADDWF   NUMERATOR
    DECF    QUOTIENT

    MOVFF   NUMERATOR, measuredTempDecimalMID
    MOVFF   QUOTIENT, measuredTempDecimalHIGH

    GOTO    _refDECIMAL

;----------------------------------
; Split ref temp into digits
;----------------------------------
_refDECIMAL:
    MOVF    refTempREG, 0
    MOVWF   NUMERATOR2

    MOVLW   TEN
    CLRF    QUOTIENT2

_D11:
    INCF    QUOTIENT2
    SUBWF   NUMERATOR2
    BC      _D11

    ADDWF   NUMERATOR2
    DECF    QUOTIENT2

    MOVFF   NUMERATOR2, refTempDecimalLOW
    MOVFF   QUOTIENT2, NUMERATOR2
    CLRF    QUOTIENT2

_D22:
    INCF    QUOTIENT2
    SUBWF   NUMERATOR2
    BC      _D22

    ADDWF   NUMERATOR2
    DECF    QUOTIENT2

    MOVFF   NUMERATOR2, refTempDecimalMID
    MOVFF   QUOTIENT2, refTempDecimalHIGH

;----------------------------------
; Stay here at the end
;----------------------------------
_END:
    GOTO    _END     // Add Breakpoint Here

    END
