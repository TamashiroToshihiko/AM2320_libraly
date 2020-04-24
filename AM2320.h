/* 
 * File:   AM2320.h
 * Author: ttama
 *
 * Created on 2019/12/10, 11:02
 */

#ifndef AM2320_H
#define	AM2320_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct _AM2320_device
    {
        unsigned char address;
        unsigned char *write_data;
        unsigned short int write_length;
        unsigned char *read_data;
        unsigned short int read_length;
        unsigned char busy;
        unsigned char on;
        float humidity;
        float temp;
    }AM2320_device;

    
    void AM2320_start_up(void) ;
    void AM2320_task(void);
    unsigned int AM2320_get_data(AM2320_device *AM2320_get);
#ifdef	__cplusplus
}
#endif

#endif	/* AM2320_H */

