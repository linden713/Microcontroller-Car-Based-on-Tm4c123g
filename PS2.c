#include "PS2.h"

volatile uint32_t usTicks = 0;           // 微秒的时钟周期数
volatile uint32_t System_Time_Ms = 0;    // 系统运行的毫秒数

void User_Delay_us(uint32_t nus)
{
    uint32_t t0 = micros();              // 记录当前的微秒数
    while (micros() - t0 < nus)          // 等待指定的微秒数
        ;
}

void SysTick_IntHandler(void)
{
    System_Time_Ms++;                    // 每次SysTick中断时，系统毫秒数加1
}

uint32_t micros(void)
{
    register uint32_t ms, tick;
    do {
        ms   = System_Time_Ms;           // 获取当前系统运行的毫秒数
        tick = HWREG(NVIC_ST_CURRENT);   // 获取当前SysTick定时器内部的计数值
    } while (ms != System_Time_Ms);       // 确保获取到的毫秒数是一致的

    return (ms + 1) * 1000 - tick / usTicks;  // 计算当前微秒数
}

void SysTick_Init(void)
{
    SysTickPeriodSet(SysCtlClockGet() / 1000);    // 设置SysTick定时器的周期为1毫秒
    SysTickIntRegister(SysTick_IntHandler);       // 注册SysTick中断处理函数
    SysTickIntEnable();                           // 启用SysTick中断
    SysTickEnable();                              // 启用SysTick定时器
    usTicks = SysCtlClockGet() / 1000000;          // 计算微秒的时钟周期数
    volatile uint32_t usTicks        = 0;          // 微秒的时钟周期数（volatile修饰符用于确保变量不被优化）
    volatile uint32_t System_Time_Ms = 0;          // 系统运行的毫秒数（volatile修饰符用于确保变量不被优化）
}


#define DELAY_TIME  User_Delay_us(5); 

//按键值读取，零时存储
u16 Handkey;
//开始命令。请求数据
u8 Comd[2]={0x01,0x42};
//数据存储数组
u8 Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; 

u16 MASK[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
    };  //按键值与按键名

/**************************************************************************
Function: Read the control of the ps2 handle
Input   : none
Output  : none
函数功能：读取PS2手柄的控制量
入口参数：无
返回  值：无
**************************************************************************/
void PS2_Read(void)
{
  //读取按键键值
    u8 PS2_KEY=PS2_DataKey();
  //读取左边遥感X轴方向的模拟量
    u8 PS2_LX=PS2_AnologData(PSS_LX);
  //读取左边遥感Y轴方向的模拟量
    u8 PS2_LY=PS2_AnologData(PSS_LY);
  //读取右边遥感X轴方向的模拟量
    u8 PS2_RX=PS2_AnologData(PSS_RX);
  //读取右边遥感Y轴方向的模拟量
    u8 PS2_RY=PS2_AnologData(PSS_RY);
}
/**************************************************************************
Function: Send commands to the handle
Input   : none
Output  : none
函数功能：向手柄发送命令
入口参数：无
返回  值：无
**************************************************************************/
void PS2_Cmd(u8 CMD)
{
    volatile u16 ref=0x01;
    Data[1] = 0;
    for(ref=0x01;ref<0x0100;ref<<=1)
    {
        if(ref&CMD)
        {
            DO_H;      //输出一位控制位
        }
        else DO_L;

        CLK_H;      //时钟拉高
        DELAY_TIME;
        CLK_L;
        DELAY_TIME;
        CLK_H;
        if(DI)
            Data[1] = ref|Data[1];
    }
    User_Delay_us(16);
}
/**************************************************************************
Function: Whether it is a red light mode, 0x41= analog green light, 0x73= analog red light
Input   : none
Output  : 0: red light mode, other: other modes
函数功能：判断是否为红灯模式,0x41=模拟绿灯，0x73=模拟红灯
入口参数：无
返回  值：0：红灯模式，其他：其他模式
**************************************************************************/
u8 PS2_RedLight(void)
{
    CS_L;
    PS2_Cmd(Comd[0]);  //开始命令
    PS2_Cmd(Comd[1]);  //请求数据
    CS_H;

    if( Data[1] == 0X73)   return 0 ;
    else return 1;

}
/**************************************************************************
Function: Read the handle data
Input   : none
Output  : none
函数功能：读取手柄数据
入口参数：无
返回  值：无
**************************************************************************/
void PS2_ReadData(void)
{
    volatile u8 byte=0;
    volatile u16 ref=0x01;
    CS_L;
    PS2_Cmd(Comd[0]);   //开始命令
    PS2_Cmd(Comd[1]);   //请求数据
    for(byte=2;byte<9;byte++)  //开始接受数据
    {
        for(ref=0x01;ref<0x100;ref<<=1)
        {
            CLK_H;
            DELAY_TIME;
            CLK_L;
            DELAY_TIME;
            CLK_H;
              if(DI)
              Data[byte] = ref|Data[byte];
        }
        User_Delay_us(16);
    }
    CS_H;
}
/**************************************************************************
Function: Handle the data of the read 2 and handle only the key parts
Input   : none
Output  : 0: only one button presses the next time; No press
函数功能：对读出来的PS2的数据进行处理,只处理按键部分
入口参数：无
返回  值：0: 只有一个按键按下时按下; 1: 未按下
**************************************************************************/
u8 PS2_DataKey()
{
    u8 index;

    PS2_ClearData();
    PS2_ReadData();

    Handkey=(Data[4]<<8)|Data[3];  //这是16个按键，按下为0，未按下为1
    for(index=0;index<16;index++)
    {
        if((Handkey&(1<<(MASK[index]-1)))==0)
        return index+1;
    }
    return 0;  //没有任何按键按下
}
/**************************************************************************
Function: Get a simulation of a rocker
Input   : Rocker
Output  : Simulation of rocker, range 0~ 256
函数功能：得到一个摇杆的模拟量
入口参数：摇杆
返回  值：摇杆的模拟量, 范围0~256
**************************************************************************/
u8 PS2_AnologData(u8 button)
{
    return Data[button];
}
/**************************************************************************
Function: Clear data buffer
Input   : none
Output  : none
函数功能：清除数据缓冲区
入口参数：无
返回  值：无
**************************************************************************/
void PS2_ClearData()
{
    u8 a;
    for(a=0;a<9;a++)
        Data[a]=0x00;
}
/******************************************************
Function: Handle vibration function
Input   : motor1: the right small vibrator, 0x00, other
          motor2: the left big shock motor 0x40~ 0xff motor is open, and the value is greater
Output  : none
函数功能：手柄震动函数
入口参数：motor1:右侧小震动电机 0x00关，其他开
            motor2:左侧大震动电机 0x40~0xFF 电机开，值越大 震动越大
返回  值：无
******************************************************/
void PS2_Vibration(u8 motor1, u8 motor2)
{
    CS_L;
    User_Delay_us(16);
  PS2_Cmd(0x01);     //开始命令
    PS2_Cmd(0x42);   //请求数据
    PS2_Cmd(0X00);
    PS2_Cmd(motor1);
    PS2_Cmd(motor2);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    CS_H;
    User_Delay_us(16);
}
/**************************************************************************
Function: Short press
Input   : none
Output  : none
函数功能：短按
入口参数：无
返回  值：无
**************************************************************************/
void PS2_ShortPoll(void)
{
    CS_L;
    User_Delay_us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x42);
    PS2_Cmd(0X00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    CS_H;
    User_Delay_us(16);
}
/**************************************************************************
Function: Enter configuration
Input   : none
Output  : none
函数功能：进入配置
入口参数：无
返回  值：无
**************************************************************************/
void PS2_EnterConfing(void)
{
  CS_L;
    User_Delay_us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43);
    PS2_Cmd(0X00);
    PS2_Cmd(0x01);
    PS2_Cmd(0x00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    CS_H;
    User_Delay_us(16);
}
/**************************************************************************
Function: Send mode Settings
Input   : none
Output  : none
函数功能：发送模式设置
入口参数：无
返回  值：无
**************************************************************************/
void PS2_TurnOnAnalogMode(void)
{
    CS_L;
    PS2_Cmd(0x01);
    PS2_Cmd(0x44);
    PS2_Cmd(0X00);
    PS2_Cmd(0x01);       //analog=0x01;digital=0x00   软件设置发送模式
    PS2_Cmd(0xEE);       //0x03锁存设置，即不可通过按键“MODE”设置模式。
                         //0xEE不锁存软件设置，可通过按键“MODE”设置模式。
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    CS_H;
    User_Delay_us(16);
}
/**************************************************************************
Function: Vibration setting
Input   : none
Output  : none
函数功能：振动设置
入口参数：无
返回  值：无
**************************************************************************/
void PS2_VibrationMode(void)
{
    CS_L;
    User_Delay_us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x4D);
    PS2_Cmd(0X00);
    PS2_Cmd(0x00);
    PS2_Cmd(0X01);
    CS_H;
    User_Delay_us(16);
}
/**************************************************************************
Function: Complete and save the configuration
Input   : none
Output  : none
函数功能：完成并保存配置
入口参数：无
返回  值：无
**************************************************************************/
void PS2_ExitConfing(void)
{
    CS_L;
    User_Delay_us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43);
    PS2_Cmd(0X00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    CS_H;
    User_Delay_us(16);
}
/**************************************************************************
Function: Handle configuration initialization
Input   : none
Output  : none
函数功能：手柄配置初始化
入口参数：无
返回  值：无
**************************************************************************/
void PS2_SetInit(void)
{
    PS2_ShortPoll();
    PS2_ShortPoll();
    PS2_ShortPoll();
    PS2_EnterConfing();       //进入配置模式
    PS2_TurnOnAnalogMode();   //“红绿灯”配置模式，并选择是否保存
    //PS2_VibrationMode();    //开启震动模式
    PS2_ExitConfing();        //完成并保存配置
}

