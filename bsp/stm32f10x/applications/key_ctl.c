#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#include "includes.h"
#include "key_ctl.h"



void iris_auto_manual_set(u8 mode);
void pelcod_call_pre_packet_send(u8 val);
void pelcod_open_close_packet_send_exptend(u8 val,u8 speed,u8 data5);

//cmd,0,stop; 1,tele,2wide; 3,far,4,near
void pelcod_zf_packet_send(u8 cmd,u8 zfspeed);

void led_pin_init(void)
{
	GPIO_InitTypeDef GPIOD_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOD_InitStructure);	

	GPIO_ResetBits(GPIOB,GPIO_Pin_0);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_2);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);	


    GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
    GPIO_Init(GPIOC, &GPIOD_InitStructure);	
    
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);	
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);		
	GPIO_ResetBits(GPIOC,GPIO_Pin_8);	
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);	

	#if 0
    GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIOD_InitStructure);	
    
	GPIO_ResetBits(GPIOC,GPIO_Pin_10);	
	GPIO_ResetBits(GPIOC,GPIO_Pin_11);		
 
	#endif
	
	 iris_auto_manual_set(0);// auto iris mode

}



#define	KEY_PORT1		GPIOA
#define	KEY_PORT2		GPIOB

void key_pin_init(void)
{

	GPIO_InitTypeDef GPIOD_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
    GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT1, &GPIOD_InitStructure);	

	GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(KEY_PORT2, &GPIOD_InitStructure);	



//������ 20180623
GPIOD_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
GPIOD_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
GPIOD_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOB, &GPIOD_InitStructure); 

	
}

u16 key_pre = 0;



u8 mode_disp_state = 0;

static u8 pb11_mode_check(void)
{
	static u8 key_state_tmp = 0;
	
	if(GPIO_ReadInputDataBit(KEY_PORT2,GPIO_Pin_11) == 0)
	{
		rt_thread_delay(RT_TICK_PER_SECOND/50);
		if(GPIO_ReadInputDataBit(KEY_PORT2,GPIO_Pin_11) == 0)
		{

			if(key_state_tmp == 0)
			{

				key_state_tmp = 1;
				mode_disp_state++;
				if(mode_disp_state>3)
					mode_disp_state = 0;

				
				pelcod_call_pre_packet_send(mode_disp_state+201);
				return 1;

			}
		}

	}
	else
	{
		key_state_tmp = 0;

	}

	return 0;
}


u8 long_key_state = 0;

#define		KEY_DELAY_CHECK_MS		40

u32 key_merge(void)
{
	u32 data = 0;

	u32 key_tmp;
	
	//PB
	data = GPIO_ReadInputData(GPIOB);
	key_tmp = (data)&0xF000;//0
	key_tmp = key_tmp>>12;
	return key_tmp;
}


u16 key_pre_2=0;
//����0Ϊ�ް��������ط�0ֵ����Ϊ��Ӧ�İ�����
static u32 pb12_13_14_15_mode_check(void)
{
	u16 i;
	u32 key_tmp;
	static u32 long_press_cnt = 0;// 50ms
	
	key_tmp = key_merge();
	for(i=0;i<4;i++)
	{
		if(((key_tmp>>i)&0x0001)==0)
		{
			rt_thread_delay(KEY_DELAY_CHECK_MS);

			key_tmp = key_merge();

			if(((key_tmp>>i)&0x0001)==0)
			{
				 
				key_pre_2 = i+1;
				return (i+1);
 			}
		}
	}

	{
		if((key_pre_2 && key_pre_2!=(i+1))||(key_pre_2 && i==20))
		{
 
			i = key_pre_2|0x8000;
			key_pre_2 = 0;
			return i;

		}
	}
	return 0;
}


static u32 pb12_13_14_15_mode_handle(void)
{
	u32 key_tmp;
	
	key_tmp = pb12_13_14_15_mode_check();
 
	if(key_tmp>=KEY_PB12 && key_tmp<=KEY_PB15)
	{
		rt_thread_delay(30);
		switch(key_tmp)
		{

		case KEY_PB12:
			pelcod_zf_packet_send(ZF_TELE,8);

			break;
		case KEY_PB13:
			pelcod_zf_packet_send(ZF_WIDE,8);

			break;	
		case KEY_PB14:
			pelcod_zf_packet_send(ZF_FAR,8);
		
			break;
		case KEY_PB15:
			pelcod_zf_packet_send(ZF_NEAR,8);
			
				break;

		default:
			break;
		}

	}
	else if(key_tmp>=0x8000)
	{
		pelcod_zf_packet_send(ZF_STOP,8);
		rt_thread_delay(30);
	}
}




//����0Ϊ�ް��������ط�0ֵ����Ϊ��Ӧ�İ�����
static u16 key_ctl_check(void)
{
	u16 data,data2;
	u16 i;
	
	static u16 key_bak=0;
	
	data = GPIO_ReadInputData(KEY_PORT1);
	data2 = GPIO_ReadInputData(KEY_PORT2);	
	data = (data>>1)&0x00ff;
	data2 = (data2>>4)&0x0f00;
	data 	+= data2;

	
	
	for(i=0;i<12;i++)
	{
		if(((data>>i)&0x0001)==0)
		{
			rt_thread_delay(3);
			data = GPIO_ReadInputData(KEY_PORT1);
			data2 = GPIO_ReadInputData(KEY_PORT2);	
			data = (data>>1)&0x00ff;
			data2 = (data2>>4)&0x0f00;
			data 	+= data2;

			if(((data>>i)&0x0001)==0)
			{
				//
				if(i==6)
				{
					if(((data>>7)&0x0001)==0)
					{
						while(1)
						{
							data = GPIO_ReadInputData(KEY_PORT1);
							data2 = GPIO_ReadInputData(KEY_PORT2);	
							data = (data>>1)&0x00ff;
							data2 = (data2>>4)&0x0f00;
							data 	+= data2;
							
							if((((data>>6)&0x0001)!=0)&&(((data>>7)&0x0001)!=0))
								break;
							rt_thread_delay(3);
						}
						if(key_pre == 0x0708)
							return 0;
						
						key_pre = 0x0708;
						return (key_pre);
					}
				}
				
				if(i==7)
				{
					if(((data>>6)&0x0001)==0)
					{
						if(key_pre == 0x0708)
							return 0;
						
						while(1)
						{
							data = GPIO_ReadInputData(KEY_PORT1);
							data2 = GPIO_ReadInputData(KEY_PORT2);	
							data = (data>>1)&0x00ff;
							data2 = (data2>>4)&0x0f00;
							data 	+= data2;
							
							if((((data>>6)&0x0001)!=0)&&(((data>>7)&0x0001)!=0))
								break;
							rt_thread_delay(3);
						}
						
						key_pre = 0x0708;
						return (key_pre);
					}
				}

				if(key_pre == (i+1))
					return 0;
				
				key_pre = i+1;
				return (i+1);
				
			}
		}
	}

	if(key_pre)
	{
		i = key_pre|0x8000;
		key_pre = 0;
		return i;

	}
	return 0;
}


const u16 led_pin[]=
{
GPIO_Pin_0,
GPIO_Pin_1,
GPIO_Pin_2,
GPIO_Pin_3,
GPIO_Pin_4,
GPIO_Pin_5,
GPIO_Pin_6,
GPIO_Pin_7,

};


extern rt_sem_t	uart1_sem;

rt_err_t rs485_recieve_check(u8 val)
{

	
	if(rt_sem_take(uart1_sem, 30) == RT_EOK)
    {
		if (command_analysis()) 
		{
            switch(command_byte)
		    {
			 	case 0x11://call preset point

					if(Rocket_fir_data == val)
						return RT_EOK;
					break;

             	default:
				break;
	   	    }

		}
	}
	return RT_ERROR;

}


u8 cmd_buff[7];

rt_sem_t rs485_return_sem;

extern rt_err_t rs485_send_data(u8* data,u16 len);
//cmd,0,stop; 1,tele,2wide; 3,far,4,near
void pelcod_zf_packet_send(u8 cmd,u8 zfspeed)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = 0xff;
	
	switch(cmd)
	{
	case 1:
		cmd_buff_private[3] = 0x20;
		cmd_buff_private[2] = 0;
		
		cmd_buff_private[4] = zfspeed;
		break;
	case 2:
		cmd_buff_private[3] = 0x40;
		cmd_buff_private[2] = 0;
		cmd_buff_private[4] = zfspeed;
		break;
	case 3:
		cmd_buff_private[3] = 0x00;
		cmd_buff_private[2] = 0x01;//
		
		cmd_buff_private[4] = zfspeed;
		break;
	case 4:
		cmd_buff_private[3] = 0x80;
		cmd_buff_private[2] = 0;
		
		cmd_buff_private[4] = zfspeed;
		break;
	case 0:
		cmd_buff_private[3] = 0x00;
		cmd_buff_private[2] = 0;
		break;
	}
	
	//cmd_buff_private[4] = 0;
	cmd_buff_private[5] = 0;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];

	rs485_send_data(cmd_buff_private,7);
	
}


void pelcod_call_pre_packet_send(u8 val)
{
	u8 cnt;
	cmd_buff[0] = 0xff;
	cmd_buff[1] = 0xff;
	cmd_buff[2] = 0;
	cmd_buff[3] = 0x07;
	cmd_buff[4] = 0;
	cmd_buff[5] = val;
	
	cmd_buff[6] = cmd_buff[1] + cmd_buff[2] + cmd_buff[3] + cmd_buff[4] + cmd_buff[5];
	rs485_send_data(cmd_buff,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff,7);
//	}
}


void pelcod_set_pre_packet_send(u8 val)
{
	u8 cnt;
	cmd_buff[0] = 0xff;
	cmd_buff[1] = 0xff;
	cmd_buff[2] = 0;
	cmd_buff[3] = 0x03;
	cmd_buff[4] = 0;
	cmd_buff[5] = val;
	
	cmd_buff[6] = cmd_buff[1] + cmd_buff[2] + cmd_buff[3] + cmd_buff[4] + cmd_buff[5];
	rs485_send_data(cmd_buff,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff,7);
//	}
}

//val: 0,open; 1,close
void pelcod_open_close_packet_send(u8 val)
{
	u8 cnt;
	cmd_buff[0] = 0xff;
	cmd_buff[1] = 0xff;
	if(val)//close
		cmd_buff[2] = 0x04;
	else
		cmd_buff[2] = 0x02;
	cmd_buff[3] = 0;
	cmd_buff[4] = 0;
	cmd_buff[5] = 0;
	
	cmd_buff[6] = cmd_buff[1] + cmd_buff[2] + cmd_buff[3] + cmd_buff[4] + cmd_buff[5];
	rs485_send_data(cmd_buff,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff,7);
//	}
}



//val: 0,open; 1,close
void pelcod_open_close_packet_send_exptend(u8 val,u8 speed,u8 data5)
{
	u8 cnt;
	
	u8 cmd_buff_private[7];
	cmd_buff_private[0] = 0xff;
	cmd_buff_private[1] = 0xff;
	if(val)//close
		cmd_buff_private[2] = 0x04;
	else
		cmd_buff_private[2] = 0x02;
	cmd_buff_private[3] = 0;
	cmd_buff_private[4] = speed;
	cmd_buff_private[5] = data5;
	
	cmd_buff_private[6] = cmd_buff_private[1] + cmd_buff_private[2] + cmd_buff_private[3] + cmd_buff_private[4] + cmd_buff_private[5];
	rs485_send_data(cmd_buff_private,7);

//	cnt=3;
//	while(cnt--)
//	{
//		if(RT_EOK == rs485_recieve_check(val))
//			break;
//		else
//			rs485_send_data(cmd_buff_private,7);
//	}
}




//val 7,7on 8off;8,7 off,8on
void led_7_8_onoff_set(u8 val)
{
	if(val>8)
		val = 8;
	if(val==7)
	{
		GPIO_WriteBit(GPIOB, led_pin[val-1], Bit_SET);
		GPIO_WriteBit(GPIOB, led_pin[val], Bit_RESET);

	}
	else
	{
		GPIO_WriteBit(GPIOB, led_pin[val-2], Bit_RESET);
		GPIO_WriteBit(GPIOB, led_pin[val-1], Bit_SET);
	}
	//pelcod_call_pre_packet_send(val+200);
}


u8 iris_auto_manual_state = 0;// Ĭ��0 Ϊ�Զ�ģʽ
void iris_auto_manual_switch(void)
{
	if(!iris_auto_manual_state)//manual iris
	{
		iris_auto_manual_state = 1;//manual
		led_7_8_onoff_set(8);

		pelcod_call_pre_packet_send(128);//manual
	}
	else
	{
		iris_auto_manual_state = 0;
		
		led_7_8_onoff_set(7);
		pelcod_set_pre_packet_send(128);

	}

}


void iris_auto_manual_set(u8 mode)
{
	if(mode)//manual iris
	{
		iris_auto_manual_state = 1;//manual
		led_7_8_onoff_set(8);

		pelcod_call_pre_packet_send(128);//manual
	}
	else
	{
		iris_auto_manual_state = 0;
		led_7_8_onoff_set(7);
		pelcod_set_pre_packet_send(128);

	}

}



void led_onoff_set(u16 val)
{
	u8 i;

	if(val > 7)
		return;
#if 0
GPIO_WriteBit(GPIOC, GPIO_Pin_10, Bit_RESET);
GPIO_WriteBit(GPIOC, GPIO_Pin_11, Bit_RESET);

#endif
	for(i=0;i<6;i++)//������ 7�ź�8�ŵ�
	{
		GPIO_WriteBit(GPIOB, led_pin[i], Bit_RESET);

	}
	GPIO_WriteBit(GPIOB, led_pin[val-1], Bit_SET);

#if 0
	if(val == 4)
		GPIO_WriteBit(GPIOC, GPIO_Pin_10, Bit_RESET);
	if
#endif
	pelcod_call_pre_packet_send(val+200);
}

void key_io_set(u16 val)
{
u16 tmp;

	switch(val)
	{
	case 9:
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
		break;
	case 10:
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		break;
	case 11:
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		break;
	case 12:
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_SET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		break;

	
	}

	if(val & 0x8000)
	{
		tmp = val&0x8000;

		if(tmp<9)
			return;
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
	}
}



void key_handle(u16 val)
{
	if(val == 0)
		return;

	
	
	if(val == 0x0708)
	{
		iris_auto_manual_switch();
	}
	else
	{
		
		if(val == 0x8708)
			return;

#if 0		
		if(val & 0x8000)
		{
			val = val&0x7fff;
		}
		else 
#endif	

		if((val < 0x8009)&&(val >= 0x8000))
			val = val&0x7fff;
		else
		{
			if(val >=9)
			{
				key_io_set(val);
				return;
			}
			else
				return;

		}
		
		if(val<9)
		{
			if(val < 7)// �ڴ˴�����������7 8
				led_onoff_set(val);
			else
			{
				if(iris_auto_manual_state)//manual mode
				{
				if(val==7)
					pelcod_open_close_packet_send_exptend(1,3,0x80);//pelcod_open_close_packet_send(1);
				else
					pelcod_open_close_packet_send_exptend(0,3,0x80);//pelcod_open_close_packet_send(0);
				}
			}
		}
		else if(val != 0)
		{
			key_io_set(val);

		}
	}
}

void rt_key_thread_entry(void* parameter)
{

	u16 k;

	key_pin_init();
	led_pin_init();
	

    while(1)
	{
		
		pb12_13_14_15_mode_handle();

		k = key_ctl_check();
		if(k)
		{
			key_handle(k);
rt_thread_delay(100);
		}











		pb11_mode_check();
		
		
		rt_thread_delay(4);
    }
}




int rt_key_ctl_init(void)
{

	
    rt_thread_t init_thread;

    init_thread = rt_thread_create("key",
                                   rt_key_thread_entry, RT_NULL,
                                   4096, 10, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}
