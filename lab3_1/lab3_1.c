#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

#define PWM_FREQUENCY 55
uint32_t time=56666;
uint32_t man_time=56666;
//void check_button();

uint32_t count=0;
uint32_t press=0;
uint32_t auto_mode=1,man_1=0,man_2=0,man_3=0;
uint32_t sw1_shrt=0,sw1_long=0,sw2_shrt=0,sw2_long=0;
uint32_t sw1_shrt_count=0;
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint8_t ui8Adjust=83;
void PortFIntHandler(void);
void man_mode1(void);
void man_mode2(void);
void man_mode3(void);
void check_man_mode(void);

int main(void)
{


	uint32_t i;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM2);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_1);
	GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_2);
	GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK)=GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE+GPIO_O_CR)|=0x01;
	//HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK)=0;
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock=SysCtlClockGet()/64;
	ui32Load=(ui32PWMClock/PWM_FREQUENCY)-1;
	PWMGenConfigure(PWM1_BASE,PWM_GEN_2,PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE,PWM_GEN_3,PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE,PWM_GEN_2,ui32Load);
	PWMGenPeriodSet(PWM1_BASE,PWM_GEN_3,ui32Load);

	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
	PWMOutputState(PWM1_BASE,PWM_OUT_5_BIT,true);
	PWMGenEnable(PWM1_BASE,PWM_GEN_2);

	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,ui8Adjust*ui32Load/1000);
	PWMOutputState(PWM1_BASE,PWM_OUT_6_BIT,true);
	//PWMGenEnable(PWM1_BASE,PWM_GEN_2);

	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,ui8Adjust*ui32Load/1000);
	PWMOutputState(PWM1_BASE,PWM_OUT_7_BIT,true);
	PWMGenEnable(PWM1_BASE,PWM_GEN_3);

	GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_FALLING_EDGE);
	GPIOIntRegister(GPIO_PORTF_BASE,PortFIntHandler);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);

	GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_FALLING_EDGE);
	GPIOIntRegister(GPIO_PORTF_BASE,PortFIntHandler);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);

	for(i=0;i<240;i++)
	{
		ui8Adjust=10+i;
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);

		SysCtlDelay(56666);
	}
	while(1)
	{
		/*//if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
		{
		//ui8Adjust--;
		//if(ui8Adjust<56)
		{
		ui8Adjust=56;
		}
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
		}
		SysCtlDelay(100000);
		//if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
		{
		//ui8Adjust++;
		//if(ui8Adjust>111)
		{
		ui8Adjust=111;
		}
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
		}
		SysCtlDelay(100000);*/
		//Blue brightness goes up

		for(i=0;i<240;i++)
		{
			ui8Adjust=10+i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,ui8Adjust*ui32Load/1000);
			//check_button();
			check_man_mode();
			SysCtlDelay(time);
		}
		//Red brightness goes down
		for(i=0;i<240;i++)
		{
			ui8Adjust=250-i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
			//check_button();
			check_man_mode();
			SysCtlDelay(time);
		}

		//Green brightness goes up
		for(i=0;i<240;i++)
		{
			ui8Adjust=10+i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,ui8Adjust*ui32Load/1000);
			//check_button();
			check_man_mode();
			SysCtlDelay(time);
		}
		//Blue brightness goes down
		for(i=0;i<240;i++)
		{
			ui8Adjust=250-i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,ui8Adjust*ui32Load/1000);
			//check_button();
			check_man_mode();
			SysCtlDelay(time);
		}
		//Red brightness goes up
		for(i=0;i<240;i++)
		{
			ui8Adjust=10+i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
			//check_button();
			check_man_mode();
			SysCtlDelay(time);
		}
		//Green brightness goes down
		for(i=0;i<240;i++)
		{
			ui8Adjust=250-i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,ui8Adjust*ui32Load/1000);
			//check_button();
			check_man_mode();
			SysCtlDelay(time);
		}
	}
}

/*void check_button()
{
	if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00))
	{
		if(time<200000)
		{
			time=time+100000;
		}
	}
	if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00))
	{
		if(time>5666)
		{
			time=time-50000;
		}
	}
}*/

void man_mode1()
{
	uint32_t loop,i;
	for(loop=0;loop<2;loop++)
	{
		SysCtlDelay(26666666);
	}
	if(man_2==1)
	{
		man_mode2();
	}
	else
	{
		man_3=0;
		man_1=1;
		man_2=0;
		//GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
		//GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,1*ui32Load/1000);
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,1*ui32Load/1000);
		while(1)
		{
			for(i=0;i<240;i++)
			{
				ui8Adjust=10+i;
				PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
				SysCtlDelay(man_time);
			}
			for(i=0;i<240;i++)
			{
				ui8Adjust=250-i;
				PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,ui8Adjust*ui32Load/1000);
				SysCtlDelay(man_time);
			}
		}
	}

}

void man_mode2()
{
	uint32_t i;
	man_3=0;
	man_1=0;
	man_2=2;
	//GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
	//GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,1*ui32Load/1000);
	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,1*ui32Load/1000);
	while(1)
	{
		for(i=0;i<240;i++)
		{
			ui8Adjust=10+i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,ui8Adjust*ui32Load/1000);
			SysCtlDelay(man_time);
		}
		for(i=0;i<240;i++)
		{
			ui8Adjust=250-i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,ui8Adjust*ui32Load/1000);
			SysCtlDelay(man_time);
		}
	}
}

void man_mode3()
{
	uint32_t i;
	man_3=3;
	man_1=0;
	man_2=0;
	//GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
	//GPIOIntDisable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_5,1*ui32Load/1000);
	PWMPulseWidthSet(PWM1_BASE,PWM_OUT_6,1*ui32Load/1000);
	while(1)
	{
		for(i=0;i<240;i++)
		{
			ui8Adjust=10+i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,ui8Adjust*ui32Load/1000);
			SysCtlDelay(man_time);
		}
		for(i=0;i<240;i++)
		{
			ui8Adjust=250-i;
			PWMPulseWidthSet(PWM1_BASE,PWM_OUT_7,ui8Adjust*ui32Load/1000);
			SysCtlDelay(man_time);
		}
	}
}

void check_man_mode()
{
	if(man_2==1)
	{
		man_mode2();
	}
	else if(man_1==1)
	{
		man_mode1();
	}
	else if(man_3==1)
	{
		man_mode3();
	}
}

void PortFIntHandler(void)
{
	//uint32_t count=0;
	count=0;
	GPIOIntClear(GPIO_PORTF_BASE,GPIO_INT_PIN_4);
	GPIOIntClear(GPIO_PORTF_BASE,GPIO_INT_PIN_0);
	/*if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)==0x0)
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,2);

	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)==0x0)
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,8);*/
	SysCtlDelay(266666);
	if((man_1||man_2||man_3)==1)
	{
		if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)==0x0)
		{
			if(man_time<260000)
			{
				man_time=man_time+50000;
			}
		}
		else if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)==0x0)
		{
			if(man_time>50000)
			{
				man_time=man_time-50000;
			}
		}
	}
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)==0x0)
	{
		while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)==0x0)
		{
			count++;
			SysCtlDelay(133333);
		}
		if(count<100)
		{
			sw2_shrt=1;
			if(time<260000)
			{
				time=time+50000;
			}
		}
		else
		{
			sw2_long=1;
			if(sw1_long==1)
			{
				man_3=1;
				return;
			}
			return;
		}

	}
	else if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)==0x0)
	{
		while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)==0x0)
		{
			count++;
			SysCtlDelay(133333);
		}
		if(count<100)
		{
			sw1_shrt=1;

			if(sw2_long==1)
			{
				sw1_shrt_count++;
				if(sw1_shrt_count==2)
				{
					man_2=1;
					man_1=0;
					return;
				}
				if(sw1_shrt_count==1)
				{
					man_1=1;
					man_2=0;
					return;
				}
			}
			else if(time>50000)
			{
				time=time-50000;
			}

		}
		else
		{
			sw1_long=1;
			if(sw2_long==1)
			{
				man_3=1;
				return;
			}
			return;
		}

	}
	else
		return;
	return 0;
}


