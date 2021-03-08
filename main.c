/*
 * File:   main.c
 * Author: vespi
 *
 * Created on July 19, 2019, 11:46 AM
 */


#include <xc.h>
#include "Functions.h"

#pragma config OSC = INTIO67 //OSC = INTIO67 specifies the Internal RC Oscillator
#pragma config WDT = OFF  
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000

char sineWave[50] = {128,144,160,175,190,203,216,227,
236,244,250,254,255,255,254,250,
244,236,227,216,203,190,175,160,
144,128,112,96,81,66,53,40,
29,20,12,6,2,0,0,2,
6,12,20,29,40,53,66,81,
96,112}; //8 bit data

char triangleWave[50] = {10,20,31,41,51,61,72,82,
92,102,113,123,133,143,154,164,
174,184,195,205,215,225,236,246,
255,246,236,225,215,205,195,184,
174,164,154,143,133,123,113,102,
92,82,72,61,51,41,31,20,
10,0};

const float Dec2AnalogConv = 0.0048828125;
unsigned int adc_result = 0;
volatile float Vout;
volatile unsigned int count;

volatile char switch1, switch2, switchValue, i, globalFlag, musicFlag;
char lcdData[16];

void main(void) {
    i = 0, musicFlag = 0;
    OSCCON = 0b01110000; //Set the Fosc of Internal Oscillator to 8MHz
    OSCTUNEbits.PLLEN = 1; //This will change the Frequency to 32MHz
    CCP1CON = 0x00; //Enables all port D pins as I/O
    
    switchPins_init(); //Pins initialized for Switch and LEDs
    adc_init();      //RA0 will be the ADC pin
    dac_init();      //SPI interface
    lcd_init();      //Initialize LCD SCREEN
    timer_init();    //Initialize timer to be off
    
    TMR1 = 0xFF6A;
    T1CONbits.TMR1ON = 1; //Timer 1 ON
    
    TMR0 = 65491; //Count for the 440HZ A4 note FIRST NOTE

    while(1){
        switch1 = PORTAbits.RA1;
        switch2 = (PORTAbits.RA2 << 1);
        switchValue = switch1 | switch2; //Value for switches
        
        ADCON0 |= 0x02;              //Set Go bit to start conversion
        while(ADCON0bits.GO == 1);   //Waits until Go bit is set to 0
        adc_result = ADRES;          //result saved into variable
        Vout = (float)adc_result * Dec2AnalogConv; //Conversion to Analog Voltage
        count = (65416 + (Vout*18));  //Set count based on ADC input (500-1000Hz)

        switch(switchValue){
            case 0x00:
                LATAbits.LATA3 = 0;
                LATAbits.LATA4 = 0;
                lcd_write("Out DC Voltage  ");
                break;
                
            case 0x01: //TRIANGLE WAVE OUTPUT
                LATAbits.LATA3 = 1;
                LATAbits.LATA4 = 0;

                lcd_write("Triangle Wave   ");
                break;

            case 0x02: //SINE WAVE OUTPUT
                LATAbits.LATA3 = 0;
                LATAbits.LATA4 = 1;

                lcd_write("Sine Wave    ");
                break;

            case 0x03: //OUTPUT LITTLE JINGLE
                LATAbits.LATA3 = 1;
                LATAbits.LATA4 = 1;
             
                if(musicFlag == 0){
                    TMR1 = 0; //This time set to play all 7 notes 10 periods each
                    T0CONbits.TMR0ON = 1; //Turn ON TIMER 0
                }
                if(musicFlag >= 139){
                    T0CONbits.TMR0ON = 0; //Turn OFF TIMER 0
                    musicFlag = 0; //Reset the flag
                }
                
                
                lcd_write("Play Jingle     ");
                break;

            default:
                break; 
        }
        
        lcd_command(0x01); //Clear command
    }
}


 void __interrupt() TimerISR(void){
    if (PIR1bits.TMR1IF){
        PIR1bits.TMR1IF = 0; //Clears TMR1 overflow
        T1CONbits.TMR1ON = 0; //Timer 1 OFF
        TMR1 = count; //Reset the count of timer1
        

        switch(switchValue){
            case 0x00:
                dac_write(0xFF);
                break;
                
            case 0x01: //TRIANGLE WAVE OUTPUT
                dac_write(triangleWave[i]);
                break;

            case 0x02: //SINE WAVE OUTPUT
                dac_write(sineWave[i]);
                break;

            case 0x03: //OUTPUT LITTLE JINGLE
                //dac_write(sineWave[i]);
                TMR1 = 0; //This time set to play all 7 notes 20 periods each
                break;

            default:
                break; 
        }
        i++;
        if( i >= 50){
            i = 0;
        }
       
        T1CONbits.TMR1ON = 1; //Timer 1 ON
    }
    
    if(INTCONbits.TMR0IF){ //This timer will be used to play the notes A4 to G5
        T0CONbits.TMR0ON = 0;   //Turn off TIMER 0
        INTCONbits.TMR0IF = 0; //Clear the Interrupt Flag bit
                
        if(musicFlag >= 0 && musicFlag <= 19){//A4 note
            TMR0 = 65414; //Count for the 440HZ A4 note
            dac_write(sineWave[i]);
        }
        if(musicFlag >= 20 && musicFlag <= 39){//B4 note
            TMR0 = 65434; //Count for the 493.88 B4 note
            dac_write(sineWave[i]);
            
        }
        if(musicFlag >= 40 && musicFlag <= 59){//C5 note
            TMR0 = 65453; //Count for the 523.25 Hz C5 Note
            dac_write(sineWave[i]);
            
        }
        if(musicFlag >= 60 && musicFlag <= 79){//D5 note
            TMR0 = 65460; //Count for the 587.33 D5 note
            dac_write(sineWave[i]);
            
        }
        if(musicFlag >= 80 && musicFlag <= 99){//E5 note
            TMR0 = 65475; //Count for the 659.25 Hz E5 note
            dac_write(sineWave[i]);
           
        }
        if(musicFlag >= 100 && musicFlag <= 119){//F5 note
            TMR0 = 65482; //Count for the 698.46 Hz F5 note
            dac_write(sineWave[i]);
            
        }
        if(musicFlag >= 120 && musicFlag <= 139){//G5 note
            TMR0 = 65494; //Count for the 783.99 Hz G5 note
            dac_write(sineWave[i]);
        }
        
        i++;
        if( i >= 50){
            musicFlag++;//Increments after outputting the entire sineWave
            i = 0;
        }
        
        T0CONbits.TMR0ON = 1; //Turn ON TIMER 0
    }
}
 
 /*
  How to actually play a tune
  * How should is the battery being connected?
  * ISSUES:
  * I forgot to connect pin 6 of the LM360 to +5V
  * Forgot to connect a resistor to pin 16 of the LCD
  * De-solder the resister on the back of the LCD
  * Several of the grounds 
  */
