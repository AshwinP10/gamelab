/*
 * LED.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table

// initialize your LEDs
void LED_Init(void){
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00000081;
    IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00000081;
    IOMUX->SECCFG.PINCM[PB20INDEX] = 0x00000081;// regular output
    GPIOA->DOE31_0 |= (1 << 17); // enable
    GPIOB->DOE31_0 |= (1 << 19);
    GPIOB->DOE31_0 |= (1 << 20);
}
// data specifies which LED to turn on
void LED_On(uint32_t data){
    GPIOB->DOUTSET31_0 = 1 << (data);
    GPIOA->DOUTSET31_0 = 1 << (data);
    // use DOUTSET31_0 register so it does not interfere with other GPIO

}

// data specifies which LED to turn off
void LED_Off(uint32_t data){
    GPIOB->DOUTCLR31_0 = 1 << (data);
    GPIOA->DOUTCLR31_0 = 1 << (data);
    // use DOUTCLR31_0 register so it does not interfere with other GPIO

}

// data specifies which LED to toggle
void LED_Toggle(uint32_t data){
    GPIOB->DOUTTGL31_0 = 1 << (data);
    GPIOA->DOUTTGL31_0 = 1 << (data);

    // use DOUTTGL31_0 register so it does not interfere with other GPIO

}
