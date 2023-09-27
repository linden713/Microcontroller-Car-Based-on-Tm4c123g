#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ints.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/systick.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "PS2.h"
#include "buct_hal.h"

#define SYSCTL_O_GPIOHBCTL 0x06C


int left_Speed = 0;             //左输入速度（PWM）
int right_Speed = 0;            //右输入速度（PWM）
int Target_Left_Speed = 0;      //左轮目标值
int Target_Right_Speed = 0;     //右轮目标值
int Obstacle1 = 0;              //障碍物检测
int Obstacle2 = 0;
int Obstacle3 = 0;

int32_t Motor1Direction;        //指示电机方向
int32_t Motor2Direction;
float Velcity_Kp = 0.96, Velcity_Ki = 1.03, Velcity_Kd = 1.01;  //PID参数
// 声明初始化PWM函数
void PWM_init(void);
// 声明初始化UART函数
void InitConsole(void);
// 声明电机前进后退函数
void left_Forward(uint32_t Percent);
void left_Backward(uint32_t Percent);
void right_Forward(uint32_t Percent);
void right_Backward(uint32_t Percent);

// 主函数
int main(void)
{
    // 设置系统时钟
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // 使能GPIOF端口
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // 使能GPIOA端口
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // 解锁PF0引脚
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= GPIO_PIN_0;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x00;

    // 使能GPIOE端口
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    // 设置GPIOE的引脚为输出引脚
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5);

    // 设置GPIOE的引脚为输入引脚
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);

    // 设置GPIOA的引脚为输入引脚
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4);


    // 初始化UART
    InitConsole();

    // 初始化SysTick定时器
    SysTick_Init();

    // 初始化PWM
    PWM_init();

    // 输出Car is ready!
    UARTprintf("Car is ready!\n");

    // 初始化QEI1
    initQEI1();

    // 初始化QEI0
    initQEI0();

    int PS2_Key;    //键值变量

    int obs_Add;    //障碍物速度加 变量
    int Add = 0;    //加速次数
    int Sub = 0;    //减速次数
    int step = 50;  //加减速步长
    int Actual_Speed0 = 0;         // Initialize the actual speed of QEI 0 to 0
    int Actual_Speed1 = 0;           // Initialize the actual speed of QEI 1 to 0

    Target_Left_Speed = 200; Target_Right_Speed = 200;      //设定目标速度

    //uint32_t t0, t1;
    while (1) {

        // 每次循环设为0
        obs_Add=0;

        Actual_Speed0 = getQEI0Speed() * 1000 / 170;     // 得到QEI 0真实值并转换数值范围
        Actual_Speed1 = getQEI1Speed() * 1000 / 170;     // 得到QEI 1真实值并转换数值范围
        //UARTprintf("d: %d, %d\n",Actual_Speed0,Actual_Speed1);

        // PID更新输出速度
        left_Speed = Velocity_FeedbackControl0(Target_Left_Speed, Actual_Speed0);
        right_Speed = Velocity_FeedbackControl1(Target_Right_Speed, Actual_Speed1);

        // 输出目标速度、实际速度和速度控制值
        UARTprintf("%d,%d\r\n", Target_Left_Speed, Actual_Speed0);

        // 获取PS2键值
        PS2_Key = PS2_DataKey();

        // 更新红外线避障传感器
        Obstacle1 = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2);
        Obstacle2 = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3);
        Obstacle3 = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4);

        if(Obstacle1==0){            // 若前方有障碍，PWM速度+100并后退
            PS2_Key=16;obs_Add=100;
        }
        else if(Obstacle2==0){      // 若右方有障碍，PWM速度+100并左转
            PS2_Key=15;obs_Add=100;
        }
        else if(Obstacle3==0){      // 若左方有障碍，PWM速度+100并右转
            PS2_Key=14;obs_Add=100;
        }

        // 更新目标速度
        Target_Left_Speed = 200 + step * Add - step * Sub + obs_Add;
        Target_Right_Speed = 200 + step * Add - step * Sub + obs_Add;


        switch (PS2_Key) {

        case 6:Add += 1; SysCtlDelay(SysCtlClockGet() / 12); break;             // 目标速度+
        case 8:Sub += 1; SysCtlDelay(SysCtlClockGet() / 12); break;             // 目标速度-

        case 13:left_Forward(left_Speed); right_Forward(right_Speed); break;    // 小车直行
        case 14:left_Forward(left_Speed); right_Backward(right_Speed); break;   //  小车右转
        case 16:left_Backward(left_Speed); right_Forward(right_Speed); break;   //  小车左转
        case 15:left_Backward(left_Speed); right_Backward(right_Speed); break;  //  小车后退
        case 0:Target_Left_Speed = 0; Target_Right_Speed = 0; left_Forward(0); right_Forward(0);left_Backward(0); right_Backward(0);    // 小车停止
        }

        // 等待QEI 0速度稳定
        waitQEI0Speed();

        // 等待QEI 1速度稳定
        waitQEI1Speed();


    }

}
void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);  // 使能 GPIOA 外设时钟
    GPIOPinConfigure(GPIO_PA0_U0RX);  // 配置 PA0 为 U0RX 引脚
    GPIOPinConfigure(GPIO_PA1_U0TX);  // 配置 PA1 为 U0TX 引脚

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);  // 使能 UART0 外设时钟
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);  // 设置 UART0 时钟源为 PIOSC
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);  // 配置 GPIOA 的引脚 0 和 1 为 UART 引脚

    UARTStdioConfig(0, 115200, 16000000);  // 配置 UART0 为标准 I/O，波特率为 115200，时钟频率为 16MHz
}

void PWM_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  // 使能 PWM1 外设时钟

    GPIOPinConfigure(GPIO_PF0_M1PWM4);  // 配置 PF0 为 M1PWM4 引脚
    GPIOPinConfigure(GPIO_PF1_M1PWM5);  // 配置 PF1 为 M1PWM5 引脚
    GPIOPinConfigure(GPIO_PF2_M1PWM6);  // 配置 PF2 为 M1PWM6 引脚
    GPIOPinConfigure(GPIO_PF3_M1PWM7);  // 配置 PF3 为 M1PWM7 引脚

    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);  // 设置引脚为 PWM 引脚

    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);  // 配置 PWM1 的生成器 2
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);  // 配置 PWM1 的生成器 3

    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, SysCtlClockGet() / 25000);  // 设置 PWM1 的生成器 2 的周期
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, SysCtlClockGet() / 25000);  // 设置 PWM1 的生成器 3 的周期

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, 0);  // 设置 M1PWM4 的初始占空比
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 0);  // 设置 M1PWM5 的初始占空比
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 0);  // 设置 M1PWM6 的初始占空比
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 0);  // 设置 M1PWM7 的初始占空比

    PWMOutputState(PWM1_BASE, PWM_OUT_4_BIT, true);  // 使能 M1PWM4 输出
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);  // 使能 M1PWM5 输出
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);  // 使能 M1PWM6 输出
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);  // 使能 M1PWM7 输出

    PWMGenEnable(PWM1_BASE, PWM_GEN_2);  // 使能 PWM1 的生成器 2
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);  // 使能 PWM1 的生成器 3
}

void left_Forward(uint32_t Percent)
{
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, 0);  // 设置 M1PWM4 的占空比为 0

    Motor1Direction = 1;  // 设置 Motor1 的方向为正向

    Percent = ((1000 - Percent) * PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 1000);  // 将 Percent 解释为 0.1%（除以 1000）

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, Percent + 1);  // 设置 M1PWM5 的占空比为 Percent + 1
}

void left_Backward(uint32_t Percent)
{
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 0);  // 设置 M1PWM5 的占空比为 0

    Motor1Direction = -1;  // 设置 Motor1 的方向为反向

    Percent = ((1000 - Percent) * PWMGenPeriodGet(PWM1_BASE, PWM_GEN_2) / 1000);  // 将 Percent 解释为 0.1%（除以 1000）

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, Percent + 1);  // 设置 M1PWM4 的占空比为 Percent + 1
}

void right_Forward(uint32_t Percent)
{
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 0);  // 设置 M1PWM6 的占空比为 0

    Motor2Direction = 1;  // 设置 Motor2 的方向为正向

    Percent = ((1000 - Percent) * PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3) / 1000);  // 将 Percent 解释为 0.1%（除以 1000）

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, Percent + 1);  // 设置 M1PWM7 的占空比为 Percent + 1
}

void right_Backward(uint32_t Percent)
{
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 0);  // 设置 M1PWM7 的占空比为 0

    Motor2Direction = -1;  // 设置 Motor2 的方向为反向

    Percent = ((1000 - Percent) * PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3) / 1000);  // 将 Percent 解释为 0.1%（除以 1000）

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, Percent + 1);  // 设置 M1PWM6 的占空比为 Percent + 1
}
int Velocity_FeedbackControl0(int TargetVelocity, int CurrentVelocity)
{
    int Bias;  // 偏差
    static int ControlVelocity0, Last_bias0, Last_last_bias0;  // 控制速度、上一次偏差、上上次偏差（静态变量）

    Bias = TargetVelocity - CurrentVelocity;  // 计算偏差

    ControlVelocity0 += Velcity_Kp * (Bias - Last_bias0);  // 增量式PID控制器的P项
    ControlVelocity0 += Velcity_Ki * Bias;  // 增量式PID控制器的I项
    ControlVelocity0 += Velcity_Kd * (Bias - 2 * Last_bias0 + Last_last_bias0);  // 增量式PID控制器的D项

    Last_bias0 = Bias;  // 更新上一次偏差
    Last_last_bias0 = Last_bias0;  // 更新上上次偏差

    if (ControlVelocity0 > 1000)  // 控制速度上限
        return 1000;
    return ControlVelocity0;
}

int Velocity_FeedbackControl1(int TargetVelocity, int CurrentVelocity)
{
    int Bias;  // 偏差
    static int ControlVelocity1, Last_bias1, Last_last_bias1;  // 控制速度、上一次偏差、上上次偏差（静态变量）

    Bias = TargetVelocity - CurrentVelocity;  // 计算偏差

    ControlVelocity1 += Velcity_Kp * (Bias - Last_bias1);  // 增量式PID控制器的P项
    ControlVelocity1 += Velcity_Ki * Bias;  // 增量式PID控制器的I项
    ControlVelocity1 += Velcity_Kd * (Bias - 2 * Last_bias1 + Last_last_bias1);  // 增量式PID控制器的D项

    Last_bias1 = Bias;  // 更新上一次偏差
    Last_last_bias1 = Last_bias1;  // 更新上上次偏差

    if (ControlVelocity1 > 1000)  // 控制速度上限
        return 1000;
    return ControlVelocity1;
}
