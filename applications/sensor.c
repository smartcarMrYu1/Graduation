/***********************************************
1、一个循环线程
2、一个周期定时器
3、定时器回调里面给线程发邮件
4、线程里面等待接受邮件，收到邮件后，读一次IO
*
*线程：输入，用于采集传感器数据
*
***********************************************/
#include <rtthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <rtdevice.h>
#include <dwin.h>

#define EVENT_W_PIN      (0x01<<0)
#define DS18B20_PIN      (0x01<<1)
#define DHT11_PIN        (0x01<<2)
#define EVENT_UNKNOW     (0x00)


static rt_mq_t sensor_mq;
static rt_timer_t sensor_timer;
static rt_uint8_t HCSR501_data1;

float ds18b20_buff[1];
float dht11_buff[2] = {0};

rt_uint16_t ds_buffer[1] = {0};  //数据显示指令
rt_uint16_t dht11_buffer1[1] = {0};  
rt_uint16_t dht11_buffer2[1] = {0};


/*********************************************
*函数名：HCSR501_timer_callback
*功能：HCSR501回调函数
*备注：
**********************************************/
static void HCSR501_timer_callback(void *parameter)   //回调函数尽量的简短，起到通知的作用
{
    rt_uint8_t HCSR501_data = EVENT_W_PIN;
    static int _tick = 0;
    
    if(_tick%8 == 0)                   
    {
        HCSR501_data |=EVENT_UNKNOW;
    } 
		if(_tick%2 == 0)
    {
        HCSR501_data |= DS18B20_PIN;
    }
    if(_tick%4 == 0)                   
    {
        HCSR501_data |= EVENT_W_PIN;
    }
    if(_tick%6 == 0)
    {
        HCSR501_data |= DHT11_PIN;
    }

    if(_tick ++ > 10000)
    {
        _tick = 0;
    }
     if(rt_mq_send(sensor_mq, &HCSR501_data, sizeof(HCSR501_data)) != RT_EOK)
     {
         rt_kprintf("F:%s L:%d err!  message quene1 full!\n,",__FUNCTION__,__LINE__);
         return;
     }
}

/*************************************************
*函数名：HCSR501_thread_entry
*功能：
*备注：
*************************************************/
static void sensor_thread_entry(void *parameter)
{
    rt_device_t pin_dev;
    rt_device_t ds18b20_dev;
    rt_device_t dht11_dev;
 
 
    pin_dev = rt_device_find("pin");
    if(pin_dev)
    {
        if(rt_device_open(pin_dev,RT_DEVICE_OFLAG_RDONLY) == RT_EOK)   //以只读方式打开  IO
        {
            struct rt_device_pin_mode pin_mode;
            
            pin_mode.pin = 4;
            pin_mode.mode = PIN_MODE_INPUT_PULLUP;
            if(rt_device_control(pin_dev,0,&pin_mode) == RT_EOK)
            {
                rt_kprintf("Hong_wai pin open success!!\n");
            }
        }
    }
    
    ds18b20_dev = rt_device_find("18b20_0");
    if(ds18b20_dev)
    {
        if(rt_device_open(ds18b20_dev,RT_DEVICE_OFLAG_RDONLY) == RT_EOK)   //以只读方式打开   DS18B20
        {
            struct rt_device_pin_mode ds18b20_pin_mode;
            
            ds18b20_pin_mode.pin = 124;
            ds18b20_pin_mode.mode = PIN_MODE_INPUT_PULLUP;
            if(rt_device_control(ds18b20_dev,0,&ds18b20_pin_mode) != RT_EOK)
            {
                rt_kprintf("ds18b20 pin open fail!!\n");
            }
        }
    }
    
    dht11_dev = rt_device_find("dht11_0");                                //DHT11
    if(dht11_dev)
    {
        if(rt_device_open(dht11_dev,RT_DEVICE_OFLAG_RDONLY) == RT_EOK)
        {
            struct rt_device_pin_mode dht11_pin_mode;
            
            dht11_pin_mode.pin = 97;
            dht11_pin_mode.mode = PIN_MODE_INPUT_PULLUP;
            if(rt_device_control(dht11_dev,0,&dht11_pin_mode) != RT_EOK)
            {
                rt_kprintf("dht11 pin open fail!!\n");
            }
        }
    }
    
    
    
    while(1)
    {
         if(rt_mq_recv(sensor_mq,&HCSR501_data1,sizeof(HCSR501_data1),RT_WAITING_FOREVER) == RT_EOK)
         {
             if(HCSR501_data1 & EVENT_W_PIN)
             {
                 if(pin_dev)
                 {
                     struct rt_device_pin_status pin_status;
                     
                     pin_status.pin = 4;
                     pin_status.status = 1;
                     if(rt_device_read(pin_dev,0,&pin_status,sizeof(pin_status)) != sizeof(pin_status))
                     {
                         rt_kprintf("pin read fail!!!!\n");
                     }
                     else
                     {
                         rt_kprintf("pin status is :%s \n",pin_status.status ? "H" : "L");
                     }
                 }
             }
             if(HCSR501_data1 & DS18B20_PIN)
             {
                 if(ds18b20_dev)
                 {
                     rt_device_read(ds18b20_dev,0,&ds18b20_buff[0],sizeof(ds18b20_buff[0]));    //读ds18b20数据
										 ds_buffer[0] = (rt_uint16_t)(ds18b20_buff[0]);
									   if(dwin_var_write(0x1010,ds_buffer,sizeof(ds_buffer)) == RT_EOK)  
											{
													rt_kprintf("ds_buffer write data success !\n");				
											}
                 }
             }
             if(HCSR501_data1 & DHT11_PIN)
             {
                 if(dht11_dev)
                 {
                     rt_device_read(dht11_dev,0,dht11_buff,sizeof(dht11_buff));
									   dht11_buffer1[0] = (rt_uint16_t)dht11_buff[0];
									   dht11_buffer2[0] = (rt_uint16_t)dht11_buff[1];
										 if(dwin_var_write(0x1003,dht11_buffer1,sizeof(ds_buffer)) == RT_EOK)  
                     {
													rt_kprintf("dht11_buffer1[1] write data success !\n");				
                     }
										 if(dwin_var_write(0x1001,dht11_buffer2,sizeof(ds_buffer)) == RT_EOK)  
                     {
													rt_kprintf("dht11_buffer2[2] write data success !\n");				
                     }
                 }
             }
         }
    }
}



int HCSR501_part_init(void)
{
	rt_thread_t sensor_tid;

	sensor_mq = rt_mq_create("sensor_mq",1024,5,RT_IPC_FLAG_FIFO);
	if(sensor_mq == RT_NULL)
	{
			rt_kprintf("F:%s L:%d err! sensor_mq create fail!\n,",__FUNCTION__,__LINE__);
			return -1;
	}

	sensor_timer = rt_timer_create("sensor_timer",HCSR501_timer_callback,RT_NULL,500,RT_TIMER_FLAG_PERIODIC);
	if(sensor_timer == RT_NULL)
	{
			rt_kprintf("F:%s L:%d err! sensor_timer create fail!\n,",__FUNCTION__,__LINE__);
			rt_mq_delete(sensor_mq);
			return -1;
	}
	
	 sensor_tid = rt_thread_create("sensor_tid",sensor_thread_entry,RT_NULL,512,22,20);
	 if(sensor_tid == RT_NULL)
	 {
			rt_mq_delete(sensor_mq);
			rt_timer_delete(sensor_timer);
			rt_kprintf("F:%s L:%d err! sensor_tid create fail!\n\n,",__FUNCTION__,__LINE__);
			return -1;
	 }

	 
	 rt_thread_startup(sensor_tid);
	 rt_timer_start(sensor_timer);
	 return 0;
}
INIT_APP_EXPORT(HCSR501_part_init);


/*总结一般步骤：
  1、首先把需要的线程、定时器、消息队列创建好
  2、如果创建失败，则删除，以释放资源
*/
