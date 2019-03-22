#ifndef __PUBLIC_DEF_H__
#define __PUBLIC_DEF_H__

#include "rtthread.h"
#include "rtdevice.h" 


// *    ∂®“Âled PIN    * /
#define LED_1  21
#define LED_2  22

void led1_light(void)
{
    rt_pin_mode(LED_1,PIN_MODE_OUTPUT);
	  rt_pin_write(LED_1,0);
}

void led2_light(void)
{
		rt_pin_mode(LED_2,PIN_MODE_OUTPUT);
	  rt_pin_write(LED_2,0);
}

#endif

