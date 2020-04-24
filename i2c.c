/*
 * File:   i2c.c
 * Author: ttama
 *
 * Created on 2019/12/09, 13:38
 */


#include "system.h"


enum _i2c_status
{
    i2c_idle=0,
    i2c_start_send,
    i2c_send_address,
    i2c_send_data,
    i2c_stop,
    i2c_noack,
    i2c_recive_address,
    i2c_recive_data
}i2c_status;

i2c_device i2c_device_save;
void init_i2c(void) 
{
    SSPCON1 = 0x00;
    SSPCON1bits.SSPM  = 0b1000;
    SSPCON1bits.SSPEN = 1;
    SSPSTATbits.SMP = 1;
    SSPADD = 0x27;//0x01;//100k
    PIR1bits.SSPIF = 0;
    PIE1bits.SSPIE = 1;
}
void i2c_interrupt_task(void)
{
    if(PIR1bits.SSPIF)
    {
        if(SSPCON2bits.ACKSTAT == 0)
        {
            i2c_device_save.ack = 1;
        }
        else
        {
            i2c_device_save.ack = 0;
        }
        
        if(SSPSTATbits.BF)
        {
            i2c_device_save.get_ok = 1;
        }
        PIR1bits.SSPIF = 0;
    }
}
unsigned short int i2c_set_data(unsigned char device_address,unsigned char *data,unsigned short int data_length)
{
    if(i2c_device_save.idle != 1)
    {
        i2c_device_save.device_address = device_address;
        i2c_device_save.device_data    = data;
        i2c_device_save.data_length    = data_length;
        i2c_device_save.idle = 1;
        i2c_status = i2c_start_send;
    }
    else
    {
        return 0;
    }
}
unsigned short int i2c_read_data(unsigned char device_address,unsigned char *data,unsigned short int data_length,unsigned char *read_data,unsigned short int read_length)
{
    if(i2c_device_save.idle != 1)
    {
        i2c_device_save.device_address = device_address | 0x01;
        i2c_device_save.device_data    = data;
        i2c_device_save.data_length    = data_length;
        i2c_device_save.get_data       = read_data;
        i2c_device_save.get_data_lenght= read_length;
        i2c_device_save.idle = 1;
        if(data_length == 0)
        {
            i2c_status = i2c_recive_address;
        }
        else
        {
            i2c_status = i2c_start_send;
        }
    }
    else
    {
        return 0;
    }
}
unsigned short int i2c_status_idle(void)
{
    return i2c_device_save.idle;
}

void i2c_task(void)
{
    unsigned char dummy;
    switch(i2c_status)
    {
        case i2c_idle:
            break;
        case i2c_start_send:
            i2c_start_bit = 1;
            while(i2c_start_bit);
            i2c_device_save.busy = 0;
            i2c_status++;
        case i2c_send_address:
            i2c_device_save.ack   = 0;
            i2c_device_save.error = 0;
            SSPBUF = i2c_device_save.device_address & 0xFE;
            i2c_status++;
        case i2c_send_data:
            if(!i2c_device_save.ack || i2c_device_save.data_length == 0)
            {
                i2c_device_save.error++;
                if(i2c_device_save.data_length == 0 )
                {
                    i2c_status = i2c_stop;
                }
                break;
            }
            i2c_device_save.ack = 0;
            i2c_device_save.error = 0;
            if(SSPSTATbits.BF)
            {
                dummy = SSPBUF;
            }
            SSPBUF = *i2c_device_save.device_data;
            *i2c_device_save.device_data++;
            i2c_device_save.data_length--;
            if(i2c_device_save.data_length == 0)
            {
                if((i2c_device_save.device_address & 0x01) == 0x01)
                {
                    i2c_status = i2c_recive_address;
                    break;
                }
                
                i2c_status++;
            }
            break;
        case i2c_recive_address:
            i2c_restart_bit = 1;
            while(i2c_restart_bit);
            i2c_device_save.ack   = 0;
            i2c_device_save.error = 0;
            SSPBUF = i2c_device_save.device_address;
            if(SSPSTATbits.BF)
            {
                dummy = SSPBUF;
            }
            i2c_status=i2c_recive_data;
            break;
        case i2c_recive_data:
            if(i2c_device_save.get_ok)
            {
                i2c_device_save.get_ok = 0;
            }
            else if(i2c_device_save.ack)
            {
                if(i2c_recive_bit == 0)
                {
                    i2c_recive_bit = 1;
                }
                else
                {
                    init_i2c();
                    i2c_status = i2c_stop;
                }
                break; 
            }
            else if(!i2c_device_save.ack)
            {
                i2c_device_save.error++;
                break;    
            }
            i2c_device_save.ack = 0;
            i2c_device_save.error = 0;
            *i2c_device_save.get_data = SSPBUF;
            *i2c_device_save.get_data++;
            i2c_device_save.get_data_lenght--;
            if(i2c_device_save.get_data_lenght == 0)
            {
                i2c_nack_en();
                i2c_status = i2c_noack;
                break;
            }
            else
            {
                i2c_ack_en();
                i2c_recive_bit = 1;
            }
            break;
        case i2c_noack:
            if(i2c_device_save.ack)
            {
                i2c_status = i2c_stop;
            }
            break;
        case i2c_stop:
            if(i2c_device_save.busy == 1 && i2c_stop_bit == 0)
            {
                i2c_status = i2c_idle;
                i2c_device_save.idle = 0;
                i2c_device_save.busy = 0;
            }
            else if(i2c_device_save.busy)
            {
                //wait
            }
            else
            {
                i2c_device_save.busy = 1;
                i2c_stop_bit = 1;
            }
            break;
    }
    if(i2c_device_save.error>10)
    {
        i2c_device_save.error = 0;
        i2c_status = i2c_stop;
    }
}