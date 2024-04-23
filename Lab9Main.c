// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Your name
// Last Modified: 12/31/2023

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // For random float generator
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include <math.h>
#include <time.h>

#define MAX_BULLETS 100
#define MAX_ASTEROIDS 20


/*
typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
*/

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}
float randomFloat(float min, float max) { // Credits: Code for random float gen found on ChatGPT
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

//Our Asteroid Language
typedef enum {English, German} Language_t;
Language_t currentLanguage = English;
typedef enum {Welcome, Credits, Score, Language, Go, Controls, GameOver, YouLose} phrase_t;
const char Welcome_English[] = "  Welcome to \n     Asteroids";
const char Welcome_German[] = "  Willkommen \n     bei Asteroiden";
const char Credits_English[] = "By Ashwin Prakash \n       and Aadhi MS";
const char Credits_German[] = "Von Aadhi MS \n      und Aadhi MS";
const char Score_English[] = "Your score is: ";
const char Score_German[] = "Dein Ergebnis ist: ";
const char Language_English[] = "English";
const char Language_German[] = "Deutsch";
const char Go_English[] = "GO!";
const char Go_German[] = "GEHEN!";
const char GameOver_English[] = "Game Over!";
const char GameOver_German[] = "Spiel vorbei!";
const char YouLose_English[] = "You Lose";
const char YouLose_German[] = "Du Verlierst";


const char *Phrases[7][2]={
                           {Welcome_English, Welcome_German},//0
                           {Credits_English, Credits_German},
                           {Score_English, Score_German},//2
                           {Language_English, Language_German},
                           {Go_English, Go_German},//4
                           {GameOver_English, GameOver_German},//5
                           {YouLose_English, YouLose_German}//6

};

uint32_t data;
float thrustx = 0;
float thrusty = 0;
uint32_t rot = 0;
uint32_t last0= 0;
uint32_t deleteasteroid = 0;
uint8_t lives = 3;
uint8_t asteroidupdate = 0;
uint8_t asteroidsdefeated = 0;
uint8_t alldead = 0;

typedef enum {dead, critical, dying, alive} status_t;

struct sprite{

    float x; // x coordinate
    float y; // y coordinate
    const uint8_t *images[8]; // ptr->image
    status_t life; // dead/alive
};
typedef struct sprite sprite_t;

sprite_t longhorn = {
    50,                         // Initial x coordinate
    80,                         // Initial y coordinate
    { logo0, logo180, logoleft, logoright, logoUL, logoDL, logoUR, logoDR}, // Image array initialization
    alive                       // Initial status
};

struct bulletsprite{

    float x; // x coordinate
    float y; // y coordinate
    const uint8_t *images[8]; // ptr->image
    float bulthrustx;
    float bulthrusty;
    status_t life; // dead/alive
};

typedef struct bulletsprite bulletsprite_t;

bulletsprite_t bullets[MAX_BULLETS];
uint8_t bulletIndex = 0;

struct asteroidsprite{

    float x; // x coordinate
    float y; // y coordinate
    const uint8_t *images[8]; // ptr->image
    float movex;
    float movey;
    status_t life; // dead/alive
};

typedef struct asteroidsprite asteroidsprite_t;

asteroidsprite_t asteroids[MAX_ASTEROIDS];
uint8_t asteroidIndex = 0;





void firebullet() {
    // Check if there's space for a new bullet
    if (bulletIndex < MAX_BULLETS) {
        // Set the new bullet's position to the longhorn's position
        bullets[bulletIndex].x = longhorn.x;
        bullets[bulletIndex].y = longhorn.y;
        // Set the bullet's velocity based on the longhorn's direction
        bullets[bulletIndex].bulthrustx = thrustx;
        bullets[bulletIndex].bulthrusty = thrusty;
        // Activate the bullet
        bullets[bulletIndex].life = alive;
        Sound_Shoot();
        // Move to the next available slot for the next bullet
        bulletIndex++;
    }
}

void createasteroid(){
    if (asteroidIndex < MAX_ASTEROIDS) {
        asteroids[asteroidIndex].life = alive;
        asteroids[asteroidIndex].x = randomFloat(5, 115); // Initialize x coordinate
        asteroids[asteroidIndex].y = randomFloat(5, 155);
        asteroids[asteroidIndex].movex = randomFloat(-0.7, 0.7);
        asteroids[asteroidIndex].movey = randomFloat(-0.7, 0.7);
        asteroidIndex++;
    }
}










// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}











// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  uint32_t theSwitch = Switch_In();

  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot

    data = ADCin();

            if (data < 455) { //1
                thrustx = 0;
                thrusty = 1;
                rot = 0;
            }
            if (data >= 455 & data <= 910) { //2
                thrustx = -0.71;
                thrusty = 0.71;
                rot = 5;

            }
            if (data > 910 & data <= 1356) { //3
                thrustx = -1;
                thrusty = 0;
                rot = 2;
            }
            if (data > 1365 & data <= 1820) { //4
                 thrustx = -0.71;
                 thrusty = -0.71;
                 rot = 4;
            }
            if (data > 1820 & data <= 2275) { //5
                 thrustx = 0;
                 thrusty = -1;
                 rot = 1;
            }
            if (data > 2275 & data <= 2730) { //6
                thrustx = 0.71;
                thrusty = -0.71;
                rot = 6;
            }
            if (data > 2730 & data <= 3185) { //7
                thrustx = 1;
                thrusty = 0;
                rot = 3;

            }
            if (data > 3185 & data <= 3640) { //8
                thrustx = 0.71;
                thrusty = 0.71;
                rot = 7;
            }

            if (data > 3640 & data <= 4095) { //9
                thrustx = 0;
                thrusty = 1;
                rot = 0;
            }



    for (uint32_t i = 0; i < MAX_BULLETS; i++){
        if (bullets[i].life == alive){
            bullets[i].x += (bullets[i].bulthrustx);
            bullets[i].y += (bullets[i].bulthrusty);


        }
        if (bullets[i].x < -10 || bullets[i].y < -10 || bullets[i].x > 130 || bullets[i].y > 170){
            bullets[i].life = dead;

        }
    }



    if ((theSwitch & (1<<28)) != 0) {
        // Check if the sprite is within the screen boundaries
        if ((longhorn.x > 5) && (longhorn.x < 115) && (longhorn.y > 5) && (longhorn.y < 155)) {
            // Calculate the new position
            float new_x = longhorn.x + 0.8*thrustx;
            float new_y = longhorn.y + 0.8*thrusty;

            // Check if the new position is within the screen boundaries
            if ((new_x > 5) && (new_x < 115) && (new_y > 5) && (new_y < 155)) {
                // Update the sprite's position
                longhorn.x = new_x;
                longhorn.y = new_y;
            }
        }
    }

    for (uint32_t a = 0; a < MAX_ASTEROIDS; a++){

            if (asteroids[a].life == alive){

                    if (asteroids[a].x <= 10) // If asteroid is on edge, change the respective velocity
                    {
                        asteroids[a].movex = randomFloat(0.3, 0.7);
                    }
                    if (asteroids[a].x >= 110)
                    {
                        asteroids[a].movex = randomFloat(-0.7, -0.3);
                    }
                    if (asteroids[a].y <= 10)
                    {
                        asteroids[a].movey = randomFloat(0.3, 0.7);
                    }
                    if (asteroids[a].y >= 150)
                    {
                        asteroids[a].movey = randomFloat(-0.7, -0.3);
                    }

                    asteroids[a].x += asteroids[a].movex; // Update new asteroid locations
                    asteroids[a].y += asteroids[a].movey;

                    float asteroidLonghornDistance = sqrt(pow(asteroids[a].x - longhorn.x, 2) + pow(asteroids[a].y - longhorn.y, 2));
                    if (asteroidLonghornDistance < 12){
                        asteroids[a].life = dead;
                        deleteasteroid = a;
                        lives--;
                        asteroidupdate = 1;

                    }
                    for (uint32_t k = 0; k < MAX_BULLETS; k++){
                        float asteroidBulletDistance = sqrt(pow(asteroids[a].x - bullets[k].x, 2) + pow(asteroids[a].y - bullets[k].y, 2));
                        if (asteroidBulletDistance < 12){
                                                asteroids[a].life = dead;
                                                deleteasteroid = a;
                                                asteroidupdate = 1;
                                                asteroidsdefeated++;

                        }
                    }
            }
        }

    uint32_t now0 = GPIOA->DIN31_0&(1<<12);
    if((now0 != 0)&&(last0 == 0)){
        firebullet();
    }
    if((now0 == 0)&&(last0 != 1)){

    }
    last0 = now0;




    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}






// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=Welcome; myPhrase<= Language; myPhrase++){
    for(Language_t myL=English; myL<= German; myL++){
         ST7735_OutString((char *)Phrases[Language][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(30000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()

  ST7735_FillScreen(ST7735_BLACK);

  //ST7735_DrawBitmap(40, 9, logo0, 10,10);
  //ST7735_DrawBitmap(62, 159, logo180, 10, 10); // player ship bottom


  /*
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  */



}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  uint32_t theSwitch = 0;
  int t = 1;
  Sound_Init();
  while(1){
    // write code to test switches and LEDs



      if (t==1){
        Sound_Shoot();
        t = 0;
      }





  }

}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  Sound_Shoot();

  while(1){

  }
}
// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures


     ST7735_SetCursor(3, 5);
     ST7735_OutString("Choose Language");
     ST7735_SetCursor(7, 8);
     ST7735_OutString((char *)Phrases[3][0]);
     ST7735_SetCursor(5, 9);
     ST7735_OutString("(Up Button)");
     ST7735_SetCursor(7, 12);
     ST7735_OutString((char *)Phrases[3][1]);
     ST7735_SetCursor(4, 13);
     ST7735_OutString("(Down Button)");
     uint32_t theSwitch = 0;
     uint32_t language = 0;
     while(1) {
         theSwitch = Switch_In();
         if((theSwitch & ((1<<28) + (1<<31))) != 0) {
             break;
         }
     }
     if((theSwitch & (1<<28)) != 0) {
         language = 0; //English
     }
     if((theSwitch & (1<<31)) != 0) {
         language = 1; //Dutch
     }
     Clock_Delay1ms(2000);
     ST7735_FillScreen(ST7735_BLACK);

     ST7735_SetCursor(4, 5);
     ST7735_OutString((char *)Phrases[0][language]);
     ST7735_SetCursor(2, 8);
     ST7735_OutString("Slidepot: <^v>");
     ST7735_SetCursor(2, 9);
     ST7735_OutString("move(^) shoot(>)");
     ST7735_SetCursor(3, 12);
     ST7735_OutString((char *)Phrases[1][language]);

     while(1) {
              theSwitch = Switch_In();
              if((theSwitch & ((1<<28) + (1<<31))) != 0) {
                  break;
              }
     }

     ST7735_FillScreen(ST7735_BLACK);
     ST7735_SetCursor(8, 8);
     ST7735_OutString("3...");
     Clock_Delay1ms(500);
     ST7735_FillScreen(ST7735_BLACK);
     ST7735_SetCursor(8, 8);
     ST7735_OutString("2...");
     Clock_Delay1ms(500);
     ST7735_FillScreen(ST7735_BLACK);
     ST7735_SetCursor(8, 8);
     ST7735_OutString("1...");
     Clock_Delay1ms(500);
     ST7735_FillScreen(ST7735_BLACK);
     ST7735_SetCursor(8, 8);
     ST7735_OutString((char *)Phrases[4][language]);
     Clock_Delay1ms(500);
     ST7735_FillScreen(ST7735_BLACK);


     LED_On(19);
     LED_On(17);
     LED_On(20);

     __enable_irq();
     //level 1
     createasteroid();
     createasteroid();
     createasteroid();
     createasteroid();


     while(1){
    // wait for semaphore




                ST7735_DrawBitmap(longhorn.x, longhorn.y, longhorn.images[rot], 10,10);
                for (uint8_t j = 0; j < MAX_BULLETS; j++){
                    if (bullets[j].life == alive){
                        ST7735_DrawBitmap(bullets[j].x+3, bullets[j].y - 5 , bullet, 3,3);
                    }

                }
                for (uint8_t l = 0; l < MAX_ASTEROIDS; l++){
                    if (asteroids[l].life == alive){
                        ST7735_DrawBitmap(asteroids[l].x, asteroids[l].y, Asteroid, 15,15);
                    }
                    if (asteroidupdate == 1){
                        ST7735_DrawBitmap(asteroids[deleteasteroid].x, asteroids[deleteasteroid].y, AsteroidExplode, 15, 15);
                        Clock_Delay1ms(30);
                        ST7735_DrawBitmap(asteroids[deleteasteroid].x, asteroids[deleteasteroid].y, AsteroidGone, 15, 15);
                        asteroidupdate = 0;
                    }

                }

                if (lives == 2){
                    LED_Off(20);


                }
                if (lives == 1){
                    LED_Off(19);

                }
                if (lives == 0){
                    LED_Off(17);
                    ST7735_FillScreen(ST7735_BLACK);
                    ST7735_SetCursor(8, 8);
                    ST7735_OutString((char *)Phrases[5][language]);
                    Clock_Delay1ms(500);
                    ST7735_FillScreen(ST7735_BLACK);

                    ST7735_SetCursor(8, 8);
                    ST7735_OutString((char *)Phrases[6][language]);
                    Clock_Delay1ms(500);
                    ST7735_FillScreen(ST7735_BLACK);
                    ST7735_SetCursor(2, 7);
                    ST7735_OutString((char *)Phrases[2][language]);
                    ST7735_SetCursor(7, 8);
                    ST7735_OutUDec(asteroidsdefeated);
                    while (1){
                        ST7735_DrawBitmap(longhorn.x, longhorn.y, longhorn.images[rot], 10,10);
                    }
                  }

                  alldead = 0;
                  for (uint32_t m = 0; m < MAX_ASTEROIDS; m++){
                      if (asteroids[m].life == alive){
                            alldead = 1;
                      }
                   }
                   if (alldead == 0){
                          break;
                   }

    }


    ST7735_FillScreen(ST7735_BLACK);
    ST7735_SetCursor(6, 7);
    ST7735_OutString("Wave 2");
    Clock_Delay1ms(500);
    ST7735_OutString("      ");
    ST7735_FillScreen(ST7735_BLACK);
    Clock_Delay1ms(500);
    createasteroid();
    createasteroid();
    createasteroid();
    createasteroid();
    createasteroid();
    createasteroid();


    while(1){
    // wait for semaphore




              ST7735_DrawBitmap(longhorn.x, longhorn.y, longhorn.images[rot], 10,10);
              for (uint8_t j = 0; j < MAX_BULLETS; j++){
                  if (bullets[j].life == alive){
                      ST7735_DrawBitmap(bullets[j].x+3, bullets[j].y - 5 , bullet, 3,3);
                  }

              }
              for (uint8_t l = 0; l < MAX_ASTEROIDS; l++){
                  if (asteroids[l].life == alive){
                      ST7735_DrawBitmap(asteroids[l].x, asteroids[l].y, Asteroid, 15,15);
                  }
                  if (asteroidupdate == 1){
                      ST7735_DrawBitmap(asteroids[deleteasteroid].x, asteroids[deleteasteroid].y, AsteroidExplode, 15, 15);
                      Clock_Delay1ms(30);
                      ST7735_DrawBitmap(asteroids[deleteasteroid].x, asteroids[deleteasteroid].y, AsteroidGone, 15, 15);
                      asteroidupdate = 0;
                  }

              }

              if (lives == 2){
                  LED_Off(20);


              }
              if (lives == 1){
                  LED_Off(19);

              }
              if (lives == 0){
                   LED_Off(17);
                   ST7735_FillScreen(ST7735_BLACK);
                   ST7735_SetCursor(8, 8);
                   ST7735_OutString((char *)Phrases[5][language]);
                   Clock_Delay1ms(500);
                   ST7735_FillScreen(ST7735_BLACK);

                   ST7735_SetCursor(8, 8);
                   ST7735_OutString((char *)Phrases[6][language]);
                   Clock_Delay1ms(500);
                   ST7735_FillScreen(ST7735_BLACK);
                   ST7735_SetCursor(2, 7);
                   ST7735_OutString((char *)Phrases[2][language]);
                   ST7735_SetCursor(7, 8);
                   ST7735_OutUDec(asteroidsdefeated);
                   while (1){
                       ST7735_DrawBitmap(longhorn.x, longhorn.y, longhorn.images[rot], 10,10);
                   }
              }
                   asteroidIndex = 0;
                   alldead = 0;
                   for (uint32_t m = 0; m < MAX_ASTEROIDS; m++){
                       if (asteroids[m].life == alive){
                           alldead = 1;
                       }
                   }
                   if (alldead == 0){
                       break;

                   }

  }


    ST7735_FillScreen(ST7735_BLACK);
    ST7735_SetCursor(6, 7);
    ST7735_OutString("Wave 3");
    Clock_Delay1ms(500);
    ST7735_OutString("      ");
    ST7735_FillScreen(ST7735_BLACK);
    Clock_Delay1ms(500);
    createasteroid();
    createasteroid();
    createasteroid();
    createasteroid();
    createasteroid();
    createasteroid();


    while(1){
    // wait for semaphore




              ST7735_DrawBitmap(longhorn.x, longhorn.y, longhorn.images[rot], 10,10);
              for (uint8_t j = 0; j < MAX_BULLETS; j++){
                  if (bullets[j].life == alive){
                      ST7735_DrawBitmap(bullets[j].x+3, bullets[j].y - 5 , bullet, 3,3);
                  }

              }
              for (uint8_t l = 0; l < MAX_ASTEROIDS; l++){
                  if (asteroids[l].life == alive){
                      ST7735_DrawBitmap(asteroids[l].x, asteroids[l].y, Asteroid, 15,15);
                  }
                  if (asteroidupdate == 1){
                      ST7735_DrawBitmap(asteroids[deleteasteroid].x, asteroids[deleteasteroid].y, AsteroidExplode, 15, 15);
                      Clock_Delay1ms(30);
                      ST7735_DrawBitmap(asteroids[deleteasteroid].x, asteroids[deleteasteroid].y, AsteroidGone, 15, 15);
                      asteroidupdate = 0;
                  }

              }

              if (lives == 2){
                  LED_Off(20);


              }
              if (lives == 1){
                  LED_Off(19);

              }
              if (lives == 0){
                   LED_Off(17);
                   ST7735_FillScreen(ST7735_BLACK);
                   ST7735_SetCursor(8, 8);
                   ST7735_OutString((char *)Phrases[5][language]);
                   Clock_Delay1ms(500);
                   ST7735_FillScreen(ST7735_BLACK);

                   ST7735_SetCursor(8, 8);
                   ST7735_OutString((char *)Phrases[6][language]);
                   Clock_Delay1ms(500);
                   ST7735_FillScreen(ST7735_BLACK);
                   ST7735_SetCursor(2, 7);
                   ST7735_OutString((char *)Phrases[2][language]);
                   ST7735_SetCursor(7, 8);
                   ST7735_OutUDec(asteroidsdefeated);
                   while (1){
                       ST7735_DrawBitmap(longhorn.x, longhorn.y, longhorn.images[rot], 10,10);
                   }
              }
                   asteroidIndex = 0;
                   alldead = 0;
                   for (uint32_t m = 0; m < MAX_ASTEROIDS; m++){
                       if (asteroids[m].life == alive){
                           alldead = 1;
                       }
                   }
                   if (alldead == 0){

                         ST7735_FillScreen(ST7735_BLACK);
                         ST7735_SetCursor(6, 7);
                         ST7735_OutString("YOU WIN!");
                         ST7735_SetCursor(5, 8);
                         ST7735_OutString((char *)Phrases[2][language]);
                         ST7735_SetCursor(7, 9);
                         ST7735_OutUDec(asteroidsdefeated);
                         Clock_Delay1ms(800000);


                   }

  }


}
