// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// Jonathan Valvano
// 11/15/2021 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"

uint32_t Index;
uint32_t periodTwo = 0;
uint32_t Size;
const uint8_t *soundPointer;






void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this
    // Not sure if this is right

        //DAC5_Init();          // Port B is DAC
        Index = 0;
        Size = 0;
        SysTick->CTRL = 0;         // disable SysTick during setup
        SysTick->LOAD = period-1;  // reload value
        SCB->SHP[1] = SCB->SHP[1]&(~0xC0000000)|priority<<30; // set priority = 1
        SysTick->VAL = 0;          // any write to current clears it
        SysTick->CTRL =7;    // enable SysTick with core clock and interrupts
}





// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5 bit DAC
void Sound_Init(void){
// write this
    DAC5_Init();
    SysTick_IntArm(7272, 0);
  
}
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
    if (Index < Size){
        DAC5_Out(*soundPointer);    // output one value each interrupt
        Index++;
        soundPointer++;
    }

	
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
// write this
    Index = 0;
    Size = count;
    soundPointer = pt;
    SysTick->VAL =0;
}
void Sound_Shoot(void){
// write this
    Sound_Start(shoot, 4080);
  
}
void Sound_Killed(void){
// write this
    Sound_Start(invaderkilled, 3377);
  
}
void Sound_Explosion(void){
// write this
    Sound_Start(explosion, 2000);
 
}

void Sound_Fastinvader1(void){
    Sound_Start(fastinvader1, 982);
  
}
void Sound_Fastinvader2(void){
  
}
void Sound_Fastinvader3(void){
  
}
void Sound_Fastinvader4(void){
  
}
void Sound_Highpitch(void){
  
}
