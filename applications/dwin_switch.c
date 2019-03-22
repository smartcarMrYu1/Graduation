/*************************************
* file:dwin_switch.c
*
* func:扫描开关状态并设置回调
*
* stat:   ing
* ************************************/
#include "rtthread.h"
#include "rtdevice.h"
#include "dwin.h"

static rt_thread_t sw_tid;
extern dwin_page_t page[PAGE_NUM];
extern dwin_button_t    login; 


rt_uint16_t power_sw_buff1 = 0;
int power_sw1 = 0;

void switch_thread_entry(void *parameter)
{
    
	  
}

int dwin_switch_init(void)
{
	  sw_tid = rt_thread_create("dwin_sw",switch_thread_entry,RT_NULL,1024,26,10);  //运行 10 tick
	  if(sw_tid == RT_NULL)
		{
				rt_kprintf("sw thread create fail F:%s ,L: %d \n",__FUNCTION__,__LINE__);
				return -1;
		}
		
		rt_thread_startup(sw_tid);
		
		return 0;
}

INIT_APP_EXPORT(dwin_switch_init);


