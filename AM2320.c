/*
 * File:   AM2320.c
 * Author: ttama
 *
 * Created on 2019/12/10, 9:50
 */


#include "system.h"

#if !defined(I2C_H)
    #error "not include i2c.h"
#endif

#define AM2320_add (0xB8)
#define AM2320_Read_byte (8)
#define Wait_time   (1000)
AM2320_device AM2320;

enum AM2320_status
{
    idle=0,
    set_register,
    pooling,
    pooling_wait,
    pooling_restart,
    busy
}AM2320_status = idle;

unsigned char send_data[10];
unsigned char read_data[10];
int pooling_wait_count;

unsigned short crc16(unsigned char *ptr, unsigned char len);


/*
 * Wake up from sleep
 */
void AM2320_start_up(void) 
{
    AM2320.address     = AM2320_add;
    AM2320.read_length = AM2320_Read_byte;
    AM2320.on = 1;
    AM2320.write_length = 3;
    i2c_set_data(AM2320.address,0,0);
}
/*
 * AM2320 control task
 * pooling this task 
 */
void AM2320_task(void)
{
    switch(AM2320_status)
    {
        case idle:
            if(!i2c_status_idle() && AM2320.on)
            {
                AM2320_status = set_register;
            }
            break;
        case set_register:
            if(!i2c_status_idle())
            {
                i2c_set_data(AM2320.address,"\x03\x00\x04",AM2320.write_length);
            }
            AM2320_status = busy;
            break;
        case busy:
            AM2320_status = pooling;
            break;
        case pooling:
            if(!i2c_status_idle())
            {
                i2c_read_data(AM2320.address,0,0,&read_data,8);
                pooling_wait_count = 0;
                AM2320_status = pooling_wait;
            }
            break;
        case pooling_wait:
            pooling_wait_count++;
            if(pooling_wait_count > Wait_time)
            {
                AM2320_status = pooling_restart;
            }
            break;
        case pooling_restart:
           if(!i2c_status_idle())
           {
                AM2320_start_up();
                AM2320_status = idle;
           }
           break;
    }
}
/*
 * if get data from AM2320 return 1
 * no data return 0
 * 
 * data push AM2320_get->humidity and AM2320_get->temp
 */
unsigned int AM2320_get_data(AM2320_device *AM2320_get)
{
    unsigned short int check_sum,check,clear;
    check_sum = crc16(&read_data,6);
    check     = read_data[6] + (read_data[7]<<8);
    if(check_sum == check)
    {
        AM2320_get->humidity = (read_data[2]*256) + (read_data[3]);
        AM2320_get->temp     = (read_data[4]*256) + (read_data[5]);
        for(clear=0;clear<8;clear++)
        {
            read_data[clear] = 0x00;
        }
        return 1;
    }
    else
    {
        return 0;
    }
    
    
}
/*
 * error check
 */
unsigned short crc16(unsigned char *ptr, unsigned char len)
{
    unsigned short crc =0xFFFF;
    unsigned char i;
    while(len--)
    {
       crc ^=*ptr++;
       for(i=0;i<8;i++)
       {
           if(crc & 0x01)
           {
               crc>>=1;
               crc^=0xA001;
           }
           else
           {
               crc>>=1;
           }
       }
    }     
    return crc;
}