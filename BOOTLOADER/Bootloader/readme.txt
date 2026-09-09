1、功能说明
    此例程展示了IAP过程的Bootloader的基本实现。

2、使用环境

    软件开发环境：KEIL MDK-ARM V5.34
                  IAR EWARM 8.50.1 
    芯片支持：
        N32H730
        N32H735
        N32H735EC
        N32H760
        N32H762
        N32H765
        N32H765EC
        N32H785
        N32H785EC
        N32H787
        
3、使用说明

    系统配置
        1、时钟源：HSI+PLL
        2、系统时钟频率：
            600MHz

    使用方法：
        1、编译后将程序下载到开发板并复位运行。
        2、复位程序，通过串口可以看到5s倒计时的log输出，在倒计时结束之前，长按WKUP(PA0)2s以上，进入升级模式。
        如果长按时间不够，对App程序的crc校验，校验通过正常启动；如果校验失败，输出错误提示信息。

4、注意事项
    无
    
1. Function description

    This example shows an implement of a bootloader in IAP.

2. Use environment

    Software development environment: KEIL MDK-ARM V5.34
                                      IAR EWARM 8.50.1 
    MCU support:
        N32H730
        N32H735
        N32H735EC
        N32H760
        N32H762
        N32H765
        N32H765EC
        N32H785
        N32H785EC
        N32H787

3. Instructions for use

    System Configuration:
        1. Clock source: HSI+PLL
        2. System Clock frequency: 
            600MHz

     Instructions:
        1. After compiling, download the program and reset, the program start running.
        2. Reset the program, and you will see a 5-second countdown on the serial port terminal. 
           Before the countdown ends, press and hold WKUP (PA0) for more than 2 seconds to enter upgrade mode.
           If the hold time is less than 2 seconds, bootloader will check the crc of application. If crc check passed, the application will start up normally,
           Otherwise, an error message will be output on the terminal.

4. Attention
    None

