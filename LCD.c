#include <reg51.h>
#include <intrins.h>
#include <stdio.h>

typedef unsigned int uint;
typedef unsigned char uchar;

/********************���Ŷ���********************/
#define LCD_DATA P0
sbit LCD_RS = P2^4;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^6;
sbit LED = P1^0;				//1Ϩ��0����
sbit KEY_1 = P1^1;			//���ٰ���
sbit KEY_2 = P1^2;			//���ٰ���
sbit KEY_3 = P1^3;			//��ͣ����
sbit MOTOR_IN = P2^0;		//���ʹ�ܶ�
sbit MOTOR_OUT = P3^3;	//��������
/************************************************/

/********************ȫ�ֱ���********************/
#define MaxSpeed 50
#define MinSpeed 20
#define GetSpeedTime 10000		//10000 * 100us = 1s
#define AdjTime 9999					//1000 * 100us = 999ms
#define DispTime 1300					//1300 * 100us = 130ms
#define PwmTime 100						//100 * 100us = 10ms
#define LedTime 2000					//2000 * 100us = 200ms
#define KeyTime 1500					//1500 * 100us = 200ms
volatile uint _100us = 0;
volatile uint Adj_100us = 0;
volatile uint Disp_100us = 0;
volatile uchar PWM_100us = 0;
volatile uint LED_100us = 0;
volatile uint KEY_100us = 0;
volatile uint DutyCycle = 50;
volatile int Laps = 0;
volatile int SetSpeed = 30;
volatile int NowSpeed = 0;
/************************************************/

/********************��������********************/
void Delay_us(uchar x);
void Delay_ms(uchar x);
void Timer0Init(void);
void Int1Init(void);
void KEY_Proc(void);
void LED_Proc(void);
void MakePWM(void);
void AdjSpeed(void);
void GetSpeed(void);
void DispSpeed(void);
void LCD_WriteCommand(uchar com);
void LCD_WriteData(uchar dat);
void LCD_Init(void);
void LCD_Show(void);
/************************************************/

/*********************������*********************/
int main(void)
{
	MOTOR_IN = 1;
	LCD_Init();
	Delay_ms(200);
	LCD_Show();
	Delay_ms(200);
	Timer0Init();
	Int1Init();
	while(1)
	{
		KEY_Proc();
		LED_Proc();
		MakePWM();
		GetSpeed();
		AdjSpeed();
		DispSpeed();
	}
}
/************************************************/

/*****************��ʱ����******************/
void Delay_us(uchar x)		//@11.0592MHz
{
	while(x--)
	{
		_nop_();
		_nop_();
		_nop_();
	}
}

void Delay_ms(uchar x)		//@11.0592MHz
{
	uchar i, j;
	while(x--)
	{
		_nop_();
		_nop_();
		i = 11;
		j = 196;
		do
		{
			while (--j);
		} while (--i);
	}
}
/************************************************/

/*******************�ⲿ�ж�1ģ��*******************/
void Int1Init(void)
{
	IT1=1;		//�����س�����ʽ�������䣩
	EX1=1;		//��INT1���ж�����	
	EA=1;			//�����ж�	
}

void Int1(void)	interrupt 2
{
	Laps ++;
}
/************************************************/

/*******************������0ģ��*******************/
void Timer0Init(void)
{
	TMOD |= 0X01;	//ѡ��Ϊ��ʱ��0ģʽ��������ʽ1������TR0��������
	TH0 = 0xFF;		//����ʱ������ֵ����ʱ100us
	TL0 = 0xA4;	
	ET0 = 1;			//�򿪶�ʱ��0�ж�����
	EA = 1;				//�����ж�
	TR0 = 1;			//�򿪶�ʱ��
}

void Timer0(void) interrupt 1
{
	TH0 = 0xFF;		//����ʱ������ֵ����ʱ100us
	TL0 = 0xA4;	
	_100us ++;
	PWM_100us ++;
	LED_100us ++;
	KEY_100us ++;
	Disp_100us ++;
	Adj_100us ++;
}
/************************************************/

/*****************����ģ��******************/
void KEY_Proc(void)
{
	if (KEY_100us >= KeyTime)
	{
		KEY_100us = 0;
		if (KEY_1 == 0)
		{
			SetSpeed --;
		}
		else if (KEY_2 == 0)
		{
			SetSpeed ++;
		}
		else if (KEY_3 == 0)
		{
			SetSpeed = 0;
		}
	}
}
/************************************************/

/*****************LEDģ��******************/
void LED_Proc(void)
{
	if (LED_100us >= LedTime)
	{
		LED_100us = 0;
		if (NowSpeed > MaxSpeed || NowSpeed < MinSpeed)
		{
			LED = ~LED;
		}
		else
		{
			LED = 1;
		}
	}
}
/************************************************/

/*****************PWM����******************/
void MakePWM(void)
{
	if (PWM_100us > PwmTime)
	{	
		PWM_100us = 0;
		MOTOR_IN = 1;
	}
	else if (PWM_100us == DutyCycle)
	{
		MOTOR_IN = 0;
	}
}
/************************************************/

/*****************���ٺ���******************/
void GetSpeed(void)
{
	if (_100us >= GetSpeedTime)
	{
		_100us = 0;
		NowSpeed = Laps;
		Laps = 0;
	}
}
/************************************************/

/*****************�ٶȵ���ģ��******************/
void AdjSpeed(void)
{
	if (Adj_100us >= AdjTime)
	{
		Adj_100us = 0;
//		if ((SetSpeed - NowSpeed) > 0)
//		{
//			DutyCycle = (PwmTime - DutyCycle) / 2 + DutyCycle;
//		}
//		else if ((NowSpeed - SetSpeed) > 0)
//		{
//			DutyCycle = DutyCycle / 2;
//		}
		if ((SetSpeed - NowSpeed) > 0)
		{
			DutyCycle ++;
			if (DutyCycle == PwmTime)
			{
				DutyCycle = PwmTime - 1;
			}
		}
		else if ((NowSpeed - SetSpeed) > 0)
		{
			DutyCycle --;
			if (DutyCycle == 0)
			{
				DutyCycle = 1;
			}
		}
		else
		{
			return;
		}
	}
}
/************************************************/

/*****************�ٶ���ʾ����******************/
void DispSpeed(void)
{
	if (Disp_100us >= DispTime)
	{
		uchar i;
		uchar Disp[2] = "  ";
		float temp;
		Disp_100us = 0;
		LCD_WriteCommand(0x89);
		temp = SetSpeed;
		sprintf(Disp, "%d ", temp);
		for(i=0; i<2; i++)
		{
			LCD_WriteData(Disp[i]); 
		}
		LCD_WriteCommand(0xC9);
		temp = NowSpeed;
		sprintf(Disp, "%d ", temp);
		for(i=0; i<2; i++)
		{
			LCD_WriteData(Disp[i]); 
		}
	}
}
/************************************************/

/*******************LCDģ��*******************/
void LCD_WriteCommand(uchar com)
{
	LCD_EN = 0;     //ʹ��
	LCD_RS = 0;	  	//ѡ��������
	LCD_RW = 0;	 	  //ѡ��д��
	
	LCD_DATA = com; //��������
	Delay_us(1);		//�ȴ������ȶ�

	LCD_EN = 1;	    //д��ʱ��
	Delay_us(5);	  //����ʱ��
	LCD_EN = 0;
}
	   
	   
void LCD_WriteData(uchar dat)
{
	LCD_EN = 0;			//ʹ������
	LCD_RS = 1;			//ѡ����������
	LCD_RW = 0;			//ѡ��д��

	LCD_DATA = dat; //д������
	Delay_us(1);

	LCD_EN = 1;   	//д��ʱ��
	Delay_us(5);  	//����ʱ��
	LCD_EN = 0;
}

void LCD_Init(void)
{
 	LCD_WriteCommand(0x38);  //����ʾ
	LCD_WriteCommand(0x0C);  //����ʾ����ʾ���
	LCD_WriteCommand(0x06);  //дһ��ָ���1
	LCD_WriteCommand(0x01);  //����
	LCD_WriteCommand(0x80);  //��������ָ�����
}

void LCD_Show(void)
{
	uchar i;
	uchar Disp[16];
	LCD_WriteCommand(0x80);
	sprintf(Disp, "SetSpeed   rad/s");
	for(i=0; i<16; i++)
	{
		LCD_WriteData(Disp[i]); 
	}
	LCD_WriteCommand(0xC0);
	sprintf(Disp, "NowSpeed   rad/s");
	for(i=0; i<16; i++)
	{
		LCD_WriteData(Disp[i]); 
	}
}
/************************************************/
