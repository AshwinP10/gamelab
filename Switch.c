/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this
    IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00040081; // S1 Up
    IOMUX->SECCFG.PINCM[PA24INDEX] = 0x00040081; // S2 Left
    IOMUX->SECCFG.PINCM[PA31INDEX] = 0x00040081; // S3 Down
    IOMUX->SECCFG.PINCM[PA12INDEX] = 0x00040081; // S4 Right
  
}
// return current state of switches
uint32_t Switch_In(void){
    // write this
    uint32_t switchRead;

    switchRead = GPIOA->DIN31_0;

    return switchRead; //Returns 32 bit value contains info on all pins, whether high or low
}
