/*
 * LED.c
 *
 *  Created on: Nov 5, 2023
 *      Author: Aadhithyavarman Manachai Shanmugham
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table

//PB20 is green LED, PB19 is yellow, and PA17 is red

// initialize your LEDs
void LED_Init(void){
    // write this
    IOMUX->SECCFG.PINCM[PB20INDEX] = 0x00000081; // Green
    IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00000081; // Yellow
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00000081; // Red
    
}

// The way data input operates right now:
// Input 1 = Red
// Input 2 = Yellow
// Input 3 = Green

// data specifies which LED to turn on

void LED_On(uint32_t data){
    // write this
    // use DOUTSET31_0 register so it does not interfere with other GPIO

    // Pin values for each color light
    uint32_t redLight = 0x00020000;
    uint32_t yellowLight = 0x00080000;
    uint32_t greenLight = 0x00100000;

    if (data == 1) // Turn on red
    {
        GPIOA->DOUT31_0 |= redLight;
    }
    if (data == 2) // Turn on yellow
    {
        GPIOB->DOUT31_0 |= yellowLight;
    }
    if (data == 2) // Turn on green
    {
        GPIOB->DOUT31_0 |= greenLight;
    }
  
}

// data specifies which LED to turn off
void LED_Off(uint32_t data){
    // write this
    // use DOUTCLR31_0 register so it does not interfere with other GPIO

    // Pin values for each color light
    uint32_t redLight = 0x00020000;
    uint32_t yellowLight = 0x00080000;
    uint32_t greenLight = 0x00100000;

    if (data == 1) // Turn off red
    {
        GPIOA->DOUT31_0 = (GPIOA->DOUT31_0 & (~redLight));
    }
    if (data == 2) // Turn off yellow
    {
        GPIOB->DOUT31_0 = (GPIOB->DOUT31_0 & (~yellowLight));
    }
    if (data == 2) // Turn off green
    {
        GPIOB->DOUT31_0 = (GPIOB->DOUT31_0 & (~greenLight));
    }

}

// data specifies which LED to toggle
void LED_Toggle(uint32_t data){
    // write this
    // use DOUTTGL31_0 register so it does not interfere with other GPIO

}
