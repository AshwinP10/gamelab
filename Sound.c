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
uint32_t period;
uint32_t priority;
uint32_t sound;





void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this
    // Not sure if this is right

    SysTick->CTRL = 0;         // disable SysTick during setup
    SysTick->LOAD = period-1;  // reload value
    SysTick->VAL = 0;          // any write to current clears it
    SCB->SHP[1] = SCB->SHP[1]&(~0xC0000000)|priority<<30; // set priority = 1

    SysTick->CTRL = 0x0007;    // enable SysTick with core clock and interrupts

}



// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5 bit DAC
void Sound_Init(void){
        DAC5_Init();
        SysTick_IntArm(7272, 1);
        SysTick->CTRL = 0;

}

volatile uint8_t *SoundSlct;
volatile uint32_t SoundNumber;

void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
    if(SoundNumber){
        DAC5_Out(*SoundSlct);
        SoundSlct++;
        SoundNumber--;
    }
    else {
        SysTick->CTRL = 0;
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

    SoundSlct = pt;
    SoundNumber = count;
    SysTick -> CTRL = 0x07;


}

void Sound_Shoot(void){
    uint8_t *shootpt;
    shootpt = &shoot[0];
    Sound_Start(shootpt,4080);

}

void Sound_Explosion(void){
    uint8_t *explosionpt;
    explosionpt = &explosion[0];
    Sound_Start(explosionpt,2000);

}


void Sound_Death(void){
    uint8_t * highpitchpt;
    highpitchpt = & highpitch[0];
    Sound_Start( highpitchpt,1802);

}
