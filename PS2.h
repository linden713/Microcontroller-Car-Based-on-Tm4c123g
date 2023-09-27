#ifndef __PS2_H__
#define __PS2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/systick.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include <stdio.h>

/* 关联引脚：
    类模拟SPI全双工同步通信
    PE15 -> PS2_DI/DAT：接收器->单片机数据接收引脚
    PE12 -> PS2_DO/CMD：单片机->接收器数据发送引脚
    PE10 -> PS2_CS：片选引脚
    PE8  -> PS2_CLK：时钟同步引脚
 */

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

extern volatile uint32_t usTicks;         // 微秒的时钟周期数
extern volatile uint32_t System_Time_Ms;  // 系统运行的毫秒数

#define PS2_DI_GPIO   GPIO_PORTE_BASE    // DI / DAT PE3
#define PS2_DI_PIN    GPIO_PIN_3

#define PS2_DO_GPIO   GPIO_PORTE_BASE    // DO / CMD PE2
#define PS2_DO_PIN    GPIO_PIN_2

#define PS2_CS_GPIO   GPIO_PORTE_BASE    // CS PE4
#define PS2_CS_PIN    GPIO_PIN_4

#define PS2_CLK_GPIO  GPIO_PORTE_BASE    // CLK PE5
#define PS2_CLK_PIN   GPIO_PIN_5

#define DI   (GPIOPinRead(PS2_DI_GPIO, PS2_DI_PIN) >> 3)   // 输入引脚

#define DO_H GPIOPinWrite(PS2_DO_GPIO, PS2_DO_PIN, PS2_DO_PIN)    // 命令高电平
#define DO_L GPIOPinWrite(PS2_DO_GPIO, PS2_DO_PIN, 0)             // 命令低电平

#define CS_H GPIOPinWrite(PS2_CS_GPIO, PS2_CS_PIN, PS2_CS_PIN)    // 片选拉高
#define CS_L GPIOPinWrite(PS2_CS_GPIO, PS2_CS_PIN, 0)             // 片选拉低

#define CLK_H GPIOPinWrite(PS2_CLK_GPIO, PS2_CLK_PIN, PS2_CLK_PIN) // 时钟高电平
#define CLK_L GPIOPinWrite(PS2_CLK_GPIO, PS2_CLK_PIN, 0)           // 时钟低电平

// 这些是按钮的常量定义
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16

#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      16

#define PSS_RX 5   // 右摇杆X轴数据
#define PSS_RY 6   // 右摇杆Y轴数据
#define PSS_LX 7   // 左摇杆X轴数据
#define PSS_LY 8   // 左摇杆Y轴数据

extern u8 Data[9];
extern u16 MASK[16];
extern u16 Handkey;
//函数功能详见.c文件
u8 PS2_RedLight(void);   
void PS2_ReadData(void); 
void PS2_Cmd(u8 CMD);
u8 PS2_DataKey(void);
u8 PS2_AnologData(u8 button); 
void PS2_ClearData(void);
void PS2_Vibration(u8 motor1, u8 motor2);

void PS2_EnterConfing(void);
void PS2_TurnOnAnalogMode(void); 
void PS2_VibrationMode(void);  
void PS2_ExitConfing(void);
void PS2_SetInit(void);
void User_Delay_us(uint32_t nus);
void SysTick_IntHandler(void);
uint32_t micros(void);
void SysTick_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __PS2_H__ */

