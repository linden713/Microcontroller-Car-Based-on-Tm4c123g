#ifndef __BUCT_HAL_H
#define __BUCT_HAL_H
#include <stdint.h>
#include "tm4c123.h"
#define GPIOA 0
#define GPIOB 1
#define GPIOC 2
#define GPIOD 3
#define GPIOE 4
#define GPIOF 5
#define INPUT 0
#define OUTPUT 1
#define ALTERNATE 2
#define ANALOG 3
#define INPUT_PULLUP 4
void delay(volatile uint32_t dly);
// GPIO helper functions
void enableGPIO(unsigned int PortNumber);
void pinMode(unsigned int PortNumber, unsigned int bit, unsigned int mode);
void digitalWrite(unsigned int PortNumber, unsigned int bit, unsigned state);
int digitalRead(unsigned int PortNumber, unsigned int bit);
void redOn();
void redOff();
void blueOn();
void blueOff();
void greenOn();
void greenOff();
int sw1Pressed();
int sw2Pressed();
// QEI1 helper functions
void initQEI0(void);
int getQEI0Position(void);
int getQEI0Speed(void);
void waitQEI0Speed(void);
// QEI1 helper functions
void initQEI1(void);
int getQEI1Position(void);
int getQEI1Speed(void);
void waitQEI1Speed(void);
// PWM Helper functions
void initCarPWM(void);
int getMotor1Velocity(void);
int getMotor2Velocity(void);
uint32_t getMotor1Speed(void);
uint32_t getMotor2Speed(void);
void setMotor1(uint32_t Percent);
void setMotor2(uint32_t Percent);


void initPWM();
void setRed(uint32_t Percent);
void setBlue(uint32_t Percent);
void setGreen(uint32_t Percent);
// ADC0 helper functions
void initADC0();
// Allows you read ain0 and ain1
uint32_t readADC(int chan);
// returns cpu temperature in Celsius * 10
uint32_t readTemperature();

// UART helper functions
#define NEWLINE 0x0d
#define LINEFEED 0x0a
void initSerial(int BaudRate);
void eputs(const char *String);
void eputi(int i); // print integer in hex
void eputd(int d); // print integer in decimal
void eputchar(char c);
char egetchar(void);
void egets(char *string, int max);
#endif
