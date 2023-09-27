#include  "tm4c123.h"
#include "buct_hal.h"
int32_t Motor1Direction,Motor2Direction;
void delay(volatile uint32_t dly)
{
    while(dly--);
}
// GPIO helper functions
void enableGPIO(unsigned int PortNumber)
{
    if (PortNumber < 6)
    {
        SYSCTL_RCGC2 = SYSCTL_RCGC2 | (1 << PortNumber); // turn on GPIO
        SYSCTL_GPIOHBCTL = SYSCTL_GPIOHBCTL | (1 << PortNumber); // turn on AHB access to GPIO
    }
}
void pinMode(unsigned int PortNumber, unsigned int bit, unsigned int mode)
{
    if (PortNumber < 6)
    {
        enableGPIO(PortNumber); // make sure the port is enabled
        volatile uint32_t *Port = (volatile uint32_t *)(GPIOA_BASE + 0x1000*PortNumber);
        if (PortNumber==GPIOF)
        {
            // Bit 0 of GPIOF is locked at boot.  Need to unlock first
            GPIOF_LOCK = 0x4C4F434B;    // password
            GPIOF_CR = 1; // Commit changes for Bit 0
        }
        switch (mode)
        {
            case INPUT: {
                *(Port+(GPIODEN>>2)) |= (1 << bit); // enable digital mode
                *(Port+(GPIOAFSEL>>2)) &= ~(1 << bit); // disable alternative function
                *(Port+(GPIODIR>>2)) &= ~(1 << bit); // configure the bit as an input
                break;
            }
            case OUTPUT: {
                *(Port+(GPIODEN>>2)) |= (1 << bit); // enable digital mode
                *(Port+(GPIOAFSEL>>2)) &= ~(1 << bit); // disable alternative function
                *(Port+(GPIODIR>>2)) |= (1 << bit); // configure the bit as an output
                break;
            }
            case ALTERNATE: {
                *(Port+(GPIODEN>>2)) |= (1 << bit); // enable digital mode (assumption)
                *(Port+(GPIOAFSEL>>2)) |= (1 << bit); // enable alternative function
                break;
            }
            case ANALOG: {
                *(Port+(GPIODEN>>2)) &= ~(1 << bit); // disable digital mode (assumption)
                *(Port+(GPIOAFSEL>>2)) &= ~(1 << bit); // disable alternative function
                *(Port+(GPIOAMSEL>>2)) |= (1 << bit); // enable analog mode
                break;
            }
            case INPUT_PULLUP: {
                *(Port+(GPIODEN>>2)) |= (1 << bit); // enable digital mode
                *(Port+(GPIOAFSEL>>2)) &= ~(1 << bit); // disable alternative function
                *(Port+(GPIODIR>>2)) &= ~(1 << bit); // configure the bit as an input
                *(Port+(GPIOPUR>>2)) |= (1 << bit); // enable pull-up resistor
                break;
            }
        }
    }
}
void digitalWrite(unsigned int PortNumber, unsigned int bit, unsigned state)
{
    if (PortNumber < 6)
    {
        volatile uint32_t *Port = (volatile uint32_t *)(GPIOA_BASE + 0x1000*PortNumber);
        if (state != 0)
        {
            *(Port + (GPIODATA>>2)) |= (1 << bit);
        }
        else
        {
            *(Port + (GPIODATA>>2)) &= ~(1 << bit);
        }
    }
}
int digitalRead(unsigned int PortNumber, unsigned int bit)
{
    if (PortNumber < 6)
    {
        volatile uint32_t *Port = (volatile uint32_t *)(GPIOA_BASE + 0x1000*PortNumber);
        uint32_t state = (*(Port + (GPIODATA>>2))) & (1 << bit);
        if (state != 0)
            state = 1;
        return state;
    }
    else
        return 0;
}
void redOn()
{
    GPIOF_DATA |= (1 << 1); // set bit 1
}
void redOff()
{
    GPIOF_DATA &= ~(1 << 1); // clear bit 1
}
void blueOn()
{
    GPIOF_DATA |= (1 << 2); // set bit 2
}
void blueOff()
{
    GPIOF_DATA &= ~(1 << 2); // clear bit 2
}
void greenOn()
{
    GPIOF_DATA |= (1 << 3); // set bit 3
}
void greenOff()
{
    GPIOF_DATA &= ~(1 << 3); // clear bit 3
}
int sw1Pressed()
{
    // returns 1 if sw1 is pressed
    if ((GPIOF_DATA & (1 << 4))==0)
        return 1;
    else
        return 0;
}

int sw2Pressed()
{
    // returns 1 if sw2 is pressed
    if ((GPIOF_DATA & (1 << 0))==0)
        return 1;
    else
        return 0;
}
// QEI0 helper functions
void initQEI0(void)
{
    // Quadrature encoder is connected to PD6 and PD7
    // These correspond to PhaseA0 and PhaseB0
    SYSCTL_RCGCQEI |= (1 << 0); // enable the clock for QEI0
    SYSCTL_RCGCGPIO |= (1 << 3); // enable GPIOD
    SYSCTL_GPIOHBCTL |= (1 << 3); // enable AHB access to GPIOD
    GPIOD_LOCK = 0x4C4F434B; // password
    GPIOD_CR = 1<<7; // Commit changes for Bit 0
    GPIOD_DEN |= (1 << 6) + (1 << 7); // digital mode for bits 5 and 6 of GPIOC
    GPIOD_AFSEL |= (1 << 6) + (1 << 7); // alternate function mode for bits 5 and 6
    GPIOD_PCTL &= ~((0x0f << 24) + (0x0f << 28)); // zero out pin control value for bits 5 and 6
    GPIOD_PCTL |= ((6 << 24) + (6 << 28)); // zero out pin control value for bits 5 and 6

    QEI0_CTL = 0x00000020;
    QEI0_LOAD = 2000000; // This sets the timing window to 0.1 second when system clock is 16MHz       TODO
                         // If you need to sample the speed more quickly then set this to
                         // a smaller value (shorter window) and adjust speed reading accordingly
    QEI0_MAXPOS = 1000;
    QEI0_CTL |= 1;

}
int getQEI0Position()
{
    return QEI0_POS;
}
int getQEI0Speed()
{
    int speed = QEI0_SPEED;
    return speed;
}
void waitQEI0Speed()
{
    // waits for the next velocity reading to complete
    while ((QEI0_RIS & (1<<1))==0);
    QEI0_ISC = (1 << 1); // clear the bit
}
// QEI1 helper functions
void initQEI1(void)
{
    // Quadrature encoder is connected to PC5 and PC6
    // These correspond to PhaseA1 and PhaseB1
    SYSCTL_RCGCQEI |= (1 << 1); // enable the clock for QEI1
    SYSCTL_RCGCGPIO |= (1 << 2); // enable GPIOC
    SYSCTL_GPIOHBCTL |= (1 << 2); // enable AHB access to GPIOC
    GPIOC_DEN |= (1 << 5) + (1 << 6); // digital mode for bits 5 and 6 of GPIOC
    GPIOC_AFSEL |= (1 << 5) + (1 << 6); // alternate function mode for bits 5 and 6
    GPIOC_PCTL &= ~((0x0f << 20) + (0x0f << 24)); // zero out pin control value for bits 5 and 6
    GPIOC_PCTL |= ((6 << 20) + (6 << 24)); // zero out pin control value for bits 5 and 6

    QEI1_CTL = 0x00000020;
    QEI1_LOAD = 2000000; // This sets the timing window to 100 ms when system clock is 16MHz       TODO
                         // If you need to sample the speed more quickly then set this to
                         // a smaller value (shorter window) and adjust speed reading accordingly
    QEI1_MAXPOS = 1000;
    QEI1_CTL |= 1;

}
int getQEI1Position()
{
    return QEI1_POS;
}
int getQEI1Speed()
{
    int speed = QEI1_SPEED;
    return speed;
}
void waitQEI1Speed()
{
    // waits for the next velocity reading to complete
    while ((QEI1_RIS & (1<<1))==0);
    QEI1_ISC = (1 << 1); // clear the bit
}
// PWM helper functions
void initCarPWM()
{
    SYSCTL_RCGCPWM |= (1 << 1); // turn on PWM1
    SYSCTL_RCGC2 = SYSCTL_RCGC2 | (1 << 5); // turn on GPIOF
    SYSCTL_GPIOHBCTL = SYSCTL_GPIOHBCTL | (1 << 5); // turn on AHB access to GPIOF

    // Will drive the LED's using PWM
    // PF0 -> pin 28 -> Motor 1 In 1
    // PF1 -> pin 29 -> Motor 1 In 2
    // PF2 -> pin 30 -> Motor 2 In 1
    // PF3 -> pin 31 -> Motor 2 In 2
    // input bits
    // Bit 0 of GPIOF is locked at boot. Need to unlock first
    GPIOF_LOCK = 0x4C4F434B; // password
    GPIOF_CR = 1; // Commit changes for Bit 0
    GPIOF_AFSEL |= (1<<3) | (1 << 2) | (1 << 1) | (1 << 0); // select alternative function for GPIOF1,2,3
    GPIOF_DEN = GPIOF_DEN | ( (1 << 3) | (1 << 2 ) | (1 << 1) | (1 << 0)); // digital mode bits 1,2,3 of GPIOF
    GPIOF_PCTL = (5 << 12) | (5 << 8) | (5 << 4) | (5 << 0);//复用功能选择为5（通常用于PWM功能）。

    PWM1_PWMENABLE |= (1 << 7)| (1 << 6) | (1 << 5) | (1 << 4);
    PWM1_PWM2LOAD = 16000000/25000; // fsw=25kHz assuming system clock is 16MHz
    PWM1_PWM3LOAD = 16000000/25000; // fsw=25kHz assuming system clock is 16MHz
    // PWM counter will count down.  When it reaches 0 the output is set to zero
    // when it counts down to the value in the CMPA or CMPB register the output is
    // driven high.  This means that the duty is proportional to the value in CMPA or CMPB
    PWM1_PWM2GENA = (2 << 0) + (3 << 6); // Drive high on match, low on zero (gen b)
    PWM1_PWM2GENB = (2 << 0) + (3 << 10); // Drive high on match, low on zero (gen b)
    PWM1_PWM3GENA = (2 << 0) + (3 << 6); // Drive high on match, low on zero (gen a)
    PWM1_PWM3GENB = (2 << 0) + (3 << 10); // Drive high on match, low on zero (gen b)
    PWM1_PWM2CMPA = 0; // initial duty of 0
    PWM1_PWM2CMPB = 0; // initial duty of 0
    PWM1_PWM3CMPA = 0; // initial duty of 0
    PWM1_PWM3CMPB = 0; // initial duty of 0
    PWM1_PWM2CTL |= (1 << 0); // enable pwm block
    PWM1_PWM3CTL |= (1 << 0); // enable pwm block
    PWM1_PWMSYNC = 0x0f; // synchronize all counters
    PWM1_PWM2DBCTL |= 1;  // Turn on complimentary outputs (PF0,PF1) use setMotor1
    PWM1_PWM3DBCTL |= 1;  // Turn on complimentary outputs (PF2,PF3) use setMotor2
    PWM1_PWM2CMPA = PWM1_PWM2LOAD >> 1; // set 50% duty initially
    PWM1_PWM3CMPA = PWM1_PWM3LOAD >> 1; // set 50% duty initially
}
int32_t Motor1Direction;
void setMotor1(uint32_t Percent)
{
    if (Percent < 500)
        Motor1Direction = -1;
    else
        Motor1Direction = 1;
    Percent = (Percent * PWM1_PWM2LOAD)/1000;    // Interpret Percent as 0.1 percent (divide by 1000) TODO
    PWM1_PWM2CMPA = Percent+1;

}
void setMotor2(uint32_t Percent)
{
    if (Percent < 500)
        Motor2Direction = -1;
    else
        Motor2Direction = 1;
    Percent = (Percent * PWM1_PWM3LOAD)/1000;   // Interpret Percent as 0.1 percent (divide by 1000) TODO
    PWM1_PWM3CMPA = Percent+1;

}
int getMotor1Velocity()
{
    int speed = QEI0_SPEED;
    speed = speed*Motor1Direction;
    return speed;
}
int getMotor2Velocity()
{
    int speed = QEI1_SPEED;
    speed = speed*Motor2Direction;
    return speed;
}
uint32_t getMotor1Speed()
{
    int speed = QEI0_SPEED;
    return speed;
}
uint32_t getMotor2Speed()
{
    int speed = QEI1_SPEED;
    return speed;
}

void setRed(uint32_t Percent)
{
    Percent = (Percent * PWM1_PWM2LOAD)/100;
    PWM1_PWM2CMPB = Percent;
}
void setBlue(uint32_t Percent)
{
    Percent = (Percent * PWM1_PWM3LOAD)/100;
    PWM1_PWM3CMPA = Percent;
}
void setGreen(uint32_t Percent)
{
    Percent = (Percent * PWM1_PWM3LOAD)/100;
    PWM1_PWM3CMPB = Percent;
}
// ADC1 helper functions
void initADC0()
{
    enableGPIO(GPIOE);
    SET_BIT(SYSCTL_RCGC0,BIT16);          // turn on ADC0
    SET_BIT(SYSCTL_RCGC2,BIT4);           // turn on GPIOE
    SET_BIT(SYSCTL_GPIOHBCTL,BIT4);       // turn on AHB access to GPIOE
// AIN0
    GPIOE_DEN   &= ~(1<<3);  // disable digital mode for PE3
    GPIOE_AMSEL |= (1 << 3); // select analog mode
// AIN1
    GPIOE_DEN   &= ~(1<<2);  // disable digital mode for PE2
    GPIOE_AMSEL |= (1 << 2); // select analog mode
    ADC0_ACTSS  = 0; // disable all sequencers
    ADC0_SSMUX3 = 0; // select channel 0 for Sequencer 3
    ADC0_ACTSS |= (1<<3); // enable sequencer 3
    ADC0_SSCTL3 = 2;
}
uint32_t readADC(int chan)
{
    ADC0_SSCTL3 = 2;  // end of sequence
    ADC0_SSMUX3 = chan;
    ADC0_PSSI = (1<<3); // start a conversion
    while(ADC0_ACTSS & (1<<16));    // wait for conversion to complete
    uint32_t result=ADC0_SSFIFO3;   // read result
    return result;  // and return it.
}
uint32_t readTemperature()
{
    ADC0_SSCTL3 = (1 << 3) + (1 << 1);  // read temperature, end of sequence
    ADC0_SSMUX3 = 0;
    ADC0_PSSI = (1<<3); // start a conversion
    while(ADC0_ACTSS & (1<<16));    // wait for conversion to complete
    uint32_t result=ADC0_SSFIFO3;   // read result
    result = 1475 - (75*33*result)/4096; // convert to temperature in Celsius * 10
    return result;  // and return it.
}
// UART helper functions
void initSerial(int BaudRate) {
    int BaudRateDivisor;
// Turn on the clock for GPIOA (uart 0 uses it) - not sure if I need this
    SET_BIT(SYSCTL_RCGC2,BIT0);          // turn on GPIOA
    SET_BIT(SYSCTL_GPIOHBCTL,BIT0);      // turn on AHB access to GPIOA
// Turn on the clock for the UART0 peripheral
    SYSCTL_RCGCUART |= BIT0;

// Ensure alternate function number for PA0 and PA1
    SET_BIT(GPIOA_AFSEL,BIT0+BIT1);
    SET_BIT(GPIOA_PUR,BIT0+BIT1);
    SET_BIT(GPIOA_DEN,BIT0+BIT1);
    BaudRateDivisor = 16000000;                // assuming 16MHz clock
    BaudRateDivisor = BaudRateDivisor / (16 * BaudRate);

    UART0_IBRD = BaudRateDivisor;
    UART0_DR=0;
    UART0_LCRH = BIT6+BIT5; // no parity, 8 data bits, 1 stop bit
    UART0_CTL = BIT8+BIT9+BIT0; // enable tx, rx and uart
}
void eputchar(char c)
{
    UART0_DR=c;
    // Wait for transmission to finish
    while(UART0_FR & BIT3);
}
char egetchar()
{
    while(UART0_FR & BIT4); // wait for a char
    return UART0_DR;
}
void eputs (const char *s)
{
    while(*s != 0)
    {
        eputchar(*s);
        s++;
    }
}
char HexDigit(int Value)
{
    if ((Value >=0) && (Value < 10))
        return Value+'0';
    else if ((Value >9) && (Value < 16))
        return Value-10 + 'A';
    else
        return 'z';
}

void eputi(int x)
{
    // Output the number over the serial port as
    // as hexadecimal string.
    char TxString[9];
    int Index=8;
    TxString[Index]=0; // terminate the string
    Index--;
    while(Index >=0)
    {
        TxString[Index]=HexDigit(x & 0x0f);
        x = x >> 4;
        Index--;
    }
    eputs(TxString);
}
void eputd(int d)
{
// Values will range from -2147483648 to +2147483647
    char TxString[12];
    int Index=11;
    TxString[Index]=0; // terminate the string
    Index--;
    if (d < 0)
    {
        TxString[0] = '-';
        d = -d;
    }
    else
    {
        TxString[0] = '+';
    }
    while(Index > 0)
    {
        TxString[Index]= (d % 10) + 48; // convert digit to ASCII
        d = d / 10;
        Index--;
    }
    eputs(TxString);
}
void egets(char *string, int max)
{
    int index = 0;
    char c;
    eputs("Enter a command\r\n");
    while(index < max)
    {
        c = egetchar();
        string[index]=c;
        index++;
        if (c==13)
        {
            break;
        }

    }
    string[index]=0;
}
