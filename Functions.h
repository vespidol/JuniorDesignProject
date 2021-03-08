/* 
 * File: Functions.h
 * Author: Victor Espidol
 * 
 * Functions throughout the whole final project
 */


#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#include <xc.h> // include processor files - each processor file is guarded.  

void switchPins_init(void);

void dac_init(void);
void dac_write(char data);

void adc_init(void);

void timer_init(void);

void lcd_command(char x);
void lcd_char(char);
void lcd_write(char x[16]);
void lcd_init(void);

#endif	/* FUNCTIONS_H */

