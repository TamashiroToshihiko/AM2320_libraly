/* 
 * File:   i2c.h
 * Author: ttama
 *
 * Created on 2019/12/09, 13:45
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct i2c_device
    {
        unsigned char device_address;
        unsigned char *device_command;
        unsigned short int command_length;
        unsigned char *device_data;
        unsigned short int data_length;
        unsigned char *get_data;
        unsigned short int get_data_lenght;
        unsigned char get_ok;
        unsigned char ack;
        unsigned char error;
        unsigned char idle;
        unsigned char busy;
    }i2c_device;

#define i2c_start_bit    (SSPCON2bits.SEN)
#define i2c_restart_bit  (SSPCON2bits.RSEN)
#define i2c_stop_bit     (SSPCON2bits.PEN)
#define i2c_recive_bit   (SSPCON2bits.RCEN)
#define i2c_ack_en()     {SSPCON2bits.ACKDT = 0;\
                          SSPCON2bits.ACKEN = 1;}
#define i2c_nack_en()    {SSPCON2bits.ACKDT = 1;\
                          SSPCON2bits.ACKEN = 1;}
 #define i2c_on_bit      (SSPCON1bits.SSPEN)   

void init_i2c(void) ;
void i2c_interrupt_task(void);
unsigned short int i2c_set_data(unsigned char device_address,unsigned char *data,unsigned short int data_length);
unsigned short int i2c_read_data(unsigned char device_address,unsigned char *data,unsigned short int data_length,unsigned char *read_data,unsigned short int read_length);
void i2c_task(void);
unsigned short int i2c_status_idle(void);

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

