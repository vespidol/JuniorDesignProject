/*
 * File:   Functions.c
 * Author: Victor Espidol
 *
 * Created on July 19, 2019, 11:48 AM
 */


#include <xc.h>
#include "Functions.h"
#define _XTAL_FREQ 8000000

void switchPins_init(void){
    TRISAbits.TRISA1 = 1; //Switch 1 as input
    TRISAbits.TRISA2 = 1; //Switch 2 as input
    
    TRISAbits.TRISA3 = 0; //LED GREEN as output;
    TRISAbits.TRISA4 = 0; //LED RED as output;
}

void dac_init(void){
     // INITIALIZE THE SPI //
    SSPCON1 = 0b00110000;   /* No Collision, No Overflow, Idle state for clock is HIGH,
                            *SCK, SDO, SDI, and SS(L) enabled
                            *SPI Master mode, clock = Fosc/4 */
    
    SSPSTAT = 0x00;   //7- Input data sampled at END of data output
                      //6- Transmit on transition from idle to active clock state
    
    //Triggers on rising edge instead of falling edge (WHY?)
    
    TRISCbits.TRISC5 = 0; //Sets SDO pin as output
    TRISCbits.TRISC3 = 0; //Sets SCK pin as output
    
    TRISAbits.TRISA5 = 0; //Sets SS pin as output
}

void dac_write(char data){
    char readData, tempData;
    
    PORTAbits.RA5 = 0; //Set Pin 5 low for (SS.L)
    
    tempData = data;
    data = data >> 4;   //Shift data to the right 4 bits
    data = data | 0xF0; //In the Form 0xFX (X is any digit)
    SSPBUF = data;      //Sends control codes and 4 MSBs of input data
    while(!SSPSTATbits.BF); //Waits till SSPBUF is full
    
    readData = SSPBUF; //This will read the SSPBUF to clear the Buffer Full bit
    
    tempData = tempData << 4;   //shift tempData to the left 4 bits
    tempData |= 0x00; //In the form 0xX0 (X is any digit) 
    SSPBUF = tempData; //Sends lower 4 bits of input data
    while(!SSPSTATbits.BF);
    
    readData = SSPBUF; //This will read the SSPBUF to clear the Buffer Full bit
    
    PORTAbits.RA5 = 1; //Set Pin 5 high for (SS.L) to turn off

    return;
}

void adc_init(void){
    ADCON1 = 0b00001110; //sets RA0 as analog, rest of PORTA is digital
    ADCON0 = 0x01;  //Enables the ADC and selects Channel 0 (AN0)
    ADCON2 = 0b10001010;  //Right Adjusted and 2 TAD acquisition time, F0sc/32
}

void timer_init(void){ //PROBABLY GOING TO HAVE TO FIX THIS
    //TIMER1
    T1CON = 0b11011000; //Timer initially off, 8 bit (2 registers), internal clock/4
    PIE1bits.TMR1IE = 1; //Enable Timer 1 Overflow interrupt
    IPR1bits.TMR1IP = 1; //TMR1 Overflow high priority interrupt
    INTCONbits.GIE = 1; //Enable global interrupts
    INTCONbits.PEIE = 1; //Enables peripheral interrupts
    
    //TIMER0
    T0CON = 0b00000000;     //Timer 0 off, 16bit, internal CLKO,
                            //increment high to low, prescaler = 2
    INTCONbits.TMR0IE = 1;  //Enable the Timer0 interrupt
    INTCON2bits.TMR0IP = 1; //TMRO overflow high priority
    
    
    //Probably start the timer in your main code
    //TMR1 = 0xFF6A;;
    //T1CONbits.TMR1ON = 1; //Timer 1 ON
}

void lcd_command(char x){
    char temp;
    temp = x;
    
    TRISDbits.TRISD0 = 0; //DB4
    TRISDbits.TRISD1 = 0; //DB5
    TRISDbits.TRISD2 = 0; //DB6
    TRISDbits.TRISD3 = 0; //DB7
    TRISDbits.TRISD4 = 0; //RS
    TRISDbits.TRISD5 = 0; //E (enable signal)
    
    PORTD = 0x00; //Clears PORTA
    
    __delay_ms(5); //delay 0 mS
    x = x >> 4;     //Shift to right 4 bits
    x = x & 0xF;    //bitwise AND with 0xF
    x = x | 0x10;   //bitwise OR with 0x80 //E
    PORTD = x; 
    
    __delay_ms(5); //delay 0 mS
    x = x & 0xF;
    PORTD = x;
    
    __delay_ms(5); //delay 0 mS
    PORTD = 0x00; 
    
    __delay_ms(5); //delay 0 mS
    x = temp;
    x = x & 0xF;
    x = x | 0x10;
    PORTD = x;
    
    __delay_ms(5); //delay 0 mS
    x = x & 0xF;
    PORTD = x;
    
    __delay_ms(5); //delay 0 mS
}

void lcd_char(char x){
    char temp;
    
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    
    temp = x;
    PORTD = 0x10; //Pin4 (RS = 1)
    
    __delay_ms(5); //delay 0 mS
    x = x >>4;
    x = x & 0xF;
    x = x | 0x30;
    PORTD = x;
    
    __delay_ms(5); //delay 0 mS
    x = x & 0x2F;
    PORTD = x;
    
    __delay_ms(5); //delay 0 mS
    PORTD = 0x10;
    
    __delay_ms(5); //delay 0 mS
    x = temp;
    x = x & 0xF;
    x = x | 0x30;
    PORTD = x;
    
    __delay_ms(5); //delay 0 mS
    x = x & 0x2F;
    PORTD = x;

    __delay_ms(5); //delay 0 mS 
}

void lcd_init(void){
    lcd_command(0x33); 
    lcd_command(0x32); 
    lcd_command(0x2C); 
    lcd_command(0x0F); //Display on
    lcd_command(0x01); //Clear command
}

void lcd_write(char x[16]){
    int i = 0;
    while(x[i] != '\0'){
        lcd_char(x[i]);
        i++;
    }
}