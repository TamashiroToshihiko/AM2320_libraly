/*
 * File:   main.c
 * Author: tama_
 *
 * Created on 2020/04/20, 13:39
 */


#include "system.h"
// PIC18F26K20 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1H
#pragma config FOSC = HSPLL     // Oscillator Selection bits (HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1))
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 18        // Brown Out Reset Voltage bits (VBOR set to 1.8 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config HFOFST = OFF      // HFINTOSC Fast Start-up (HFINTOSC starts clocking the CPU without waiting for the oscillator to stablize.)
#pragma config MCLRE = OFF       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection Block 0 (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection Block 1 (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection Block 2 (Block 2 (008000-00BFFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection Block 3 (Block 3 (00C000-00FFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (00C000h-00FFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

void put_byte(char data);
void uart_send(char *data,int len);
char floatToASCII(char *data,float val);

AM2320_device am2320;
char buff[20];
void __interrupt () System_interrupt (void)
{
    i2c_interrupt_task();
}
void main(void) 
{
    short int baudrate,i;
    unsigned char Hello[] = "Hello!";
    //port 
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;
    LATCbits.LATC6   = 0;
    //uart
    TXSTA   = 0x20;
    RCSTA   = 0x90;
    BAUDCON = 0x08;
    baudrate = (_XTAL_FREQ / 9600 / 16 )-1;
    SPBRGH  =  (unsigned char)(baudrate>>8);
    SPBRG   =  (unsigned char)(baudrate & 0xFF);
    init_i2c();
    GIE  = 1;
    PEIE = 1;
    AM2320_start_up();
    
    while(1)
    {
        __delay_ms(10);
        
        //pooling
        i2c_task();
        AM2320_task();
        if(AM2320_get_data(&am2320))
        {
    
            
            uart_send("temp=",5);
            floatToASCII(buff,am2320.temp/10);
            uart_send(buff,strlen(buff));
            uart_send("\r\n",2);
            uart_send("hum=",4);
            floatToASCII(buff,am2320.humidity/10);
            uart_send(buff,strlen(buff));
            uart_send("\r\n",2);
            
        }
        
    }
}
void put_byte(char data)
{
     while(!TXSTAbits.TRMT);
     TXREG = data;
}
void uart_send(char *data,int len)
{
    int i;
    for(i=0;i<len;i++)
    {
        put_byte(*data++);
    }
}
char floatToASCII(char *data,float val)
{
    unsigned char get,i,sd;
    get = val;
    sd = get;
    if(get>9)
    {
        *data = (get/10)+0x30;
        *data++;
        get = get - (get/10)*10;
    }
    *data = get+0x30;
    *data++;
    *data = '.';
    *data++;
    get = (val * 10) - (sd * 10);
    *data = get+0x30;
    *data++;
    *data = 0x00;
    return 1;
}
