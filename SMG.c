#include <reg51.h>
#include <intrins.h>
#include <absacc.h>

typedef unsigned int uint;
typedef unsigned char uchar;

/********************��������********************/
#define Ctrl XBYTE[0x0FF20]
#define PA XBYTE[0x0FF21]
#define PB XBYTE[0x0FF22]
sbit LED = P1^0;				//1Ϩ��0����
sbit KEY_1 = P1^1;			//���ٰ���
sbit KEY_2 = P1^2;			//���ٰ���
sbit KEY_3 = P1^3;			//��ͣ����
sbit MOTOR_IN = P2^0;		//���ʹ�ܶ�
sbit MOTOR_OUT = P3^3;	//��������
/************************************************/

/********************ȫ�ֱ���********************/
#define MaxSpeed 30
#define MinSpeed 10
#define GetSpeedTime 1000			//1000 * 1ms = 1s
#define AdjTime 999						//999 * 1ms = 999ms
#define DispTime 4						//4 * 1ms * 6 = 24ms
#define PwmTime 10						//10 * 1ms = 10ms
#define LedTime 200						//200 * 1ms = 200ms
#define KeyTime 150						//150 * 1ms = 150ms
volatile uint _1ms = 0;
volatile uint Adj_1ms = 0;
volatile uchar Disp_1ms = 0;
volatile uchar PWM_1ms = 0;
volatile uchar LED_1ms = 0;
volatile uchar KEY_1ms = 0;
volatile uchar Duty_Cycle = 99;
volatile int Laps = 0;
volatile int SetSpeed = 40;
volatile int NowSpeed = 0;
volatile uchar Butter[6];
volatile uchar code Discode[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0x0F8, 0x80, 0x90};
/************************************************/

/********************��������********************/
void Delay_ms(uchar x);
void Timer0Init(void);
void Int1Init(void);
void KEY_Proc(void);
void LED_Proc(void);
void MakePWM(void);
void AdjSpeed(void);
void GetSpeed(void);
void DispSpeed(void);
void Display(void);
/************************************************/

/*********************������*********************/
int main(void)
{
	MOTOR_IN = 1;
	Delay_ms(200);
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
	TH0 = 0xFC;		//����ʱ������ֵ����ʱ1ms
	TL0 = 0x64;	
	ET0 = 1;			//�򿪶�ʱ��0�ж�����
	EA = 1;				//�����ж�
	TR0 = 1;			//�򿪶�ʱ��
}

void Timer0(void) interrupt 1
{
	TH0 = 0xFC;		//����ʱ������ֵ����ʱ1ms
	TL0 = 0x64;	
	_1ms ++;
	PWM_1ms ++;
	LED_1ms ++;
	KEY_1ms ++;
	Disp_1ms ++;
	Adj_1ms ++;
}
/************************************************/

/*****************����ģ��******************/
void KEY_Proc(void)
{
	if (KEY_1ms >= KeyTime)
	{
		KEY_1ms = 0;
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
	if (LED_1ms >= LedTime)
	{
		LED_1ms = 0;
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
	if (PWM_1ms >= PwmTime)
	{	
		PWM_1ms = 0;
		MOTOR_IN = 1;
	}
	else if (PWM_1ms == Duty_Cycle)
	{
		MOTOR_IN = 0;
	}
}
/************************************************/

/*****************���ٺ���******************/
void GetSpeed(void)
{
	if (_1ms >= GetSpeedTime)
	{
		_1ms = 0;
		NowSpeed = Laps;
		Laps = 0;
		Butter[5] = 0xFF;
		Butter[4] = Discode[SetSpeed / 10];
		Butter[3] = Discode[SetSpeed % 10];
		Butter[2] = 0xFF;
		Butter[1] = Discode[NowSpeed / 10];
		Butter[0] = Discode[NowSpeed % 10];
	}
}
/************************************************/

/*****************�ٶȵ���ģ��******************/
void AdjSpeed(void)
{
	if (Adj_1ms >= AdjTime)
	{
		Adj_1ms = 0;
		if ((NowSpeed - SetSpeed) > 1)
		{
			Duty_Cycle ++;
			if (Duty_Cycle == 100)
			{
				Duty_Cycle = 99;
			}
		}
		else if ((SetSpeed - NowSpeed) > 1)
		{
			Duty_Cycle --;
			if (Duty_Cycle == 0)
			{
				Duty_Cycle = 1;
			}
		}
	}
}
/************************************************/

/*****************�������ʾģ��******************/
void DispSpeed(void)
{
	if (Disp_1ms >= DispTime)
	{
		Disp_1ms = 0;
		Display();
	}
}

void Display(void)
{
	static Disp = 0;
	Ctrl = 0x43;
	switch(Disp)
	{
		case 0 :
		{
			PA = 0xFE;
			PB = Butter[0];
			Disp ++;
			break;
		}
		case 1 :
		{
			PA = 0xFD;
			PB = Butter[1];
			Disp ++;
			break;
		}
		case 2 :
		{
			PA = 0xFB;
			PB = Butter[2];
			Disp ++;
			break;
		}
		case 3 :
		{
			PA = 0xF7;
			PB = Butter[3];
			Disp ++;
			break;
		}
		case 4 :
		{
			PA = 0xEF;
			PB = Butter[4];
			Disp ++;
			break;
		}
		case 5 :
		{
			PA = 0xDF;
			PB = Butter[5];
			Disp = 0;
			break;
		}
	}
}
/************************************************/