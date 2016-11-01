#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/fpu.h"
#include "utils/uartstdio.h"
#include "one.h"
#include "two.h"
#include "three.h"
#include "four.h"
#include "five.h"
#include "six.h"
#include "seven.h"
#include "eight.h"

void glcd_init();
void glcd_cleardisplay();
void glcd_cmd(unsigned char );
void glcd_data(unsigned char );
void glcd_setpage(unsigned char );
void glcd_setcolumn(unsigned char );


uint32_t ui32ADC0Value[4]; //create an array that will be used for storing the data read from the ADC FIFO
volatile uint32_t avg;
//volatile uint32_t ui32Value1,ui32Value2;
uint32_t ui32ADC1Value[4];
volatile uint32_t x ,delay;



void glcd_cmd(unsigned char cmd)
{
	/*clear data lines */
	GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0 |GPIO_PIN_1|GPIO_PIN_2 |GPIO_PIN_3, 0x00);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_6 |GPIO_PIN_7, 0x00);

	/*RS =0*/
	GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6, 0x00);

	/*Put command on data lines */
	GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0 |GPIO_PIN_1|GPIO_PIN_2 |GPIO_PIN_3, cmd);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_6 |GPIO_PIN_7, cmd);

	/*Generate a high to low pulse on enable */
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0x01);
	SysCtlDelay(1340);
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0x00);

}

void glcd_init()
{
	SysCtlDelay(134000);
	/*clear RST*/
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,0x00);
	SysCtlDelay(134000);


	/*Set RST */
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0x20);

	/*Initialise left side of GLCD*/
	/*Set CS1(CS1=1 and CS2=0) */
	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3,0x00);

	/*Select start line */
	glcd_cmd(0xC0);
	/*Select the page*/
	glcd_cmd(0xB8);
	/*Select the column*/
	glcd_cmd(0x40);
	/*Send glcd on command*/
	glcd_cmd(0x3F);


	/*Initialise left side of GLCD*/
	/*Set CS2(CS1=0 and CS2=1) */
	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3,0x08);

	/*Select start line */
	glcd_cmd(0xC0);
	/*Select the page*/
	glcd_cmd(0xB8);
	/*Select the column*/
	glcd_cmd(0x40);
	/*Send glcd on command*/
	glcd_cmd(0x3F);

}

void glcd_data(unsigned char data)
{
	/*clear data lines */
	GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0 |GPIO_PIN_1|GPIO_PIN_2 |GPIO_PIN_3, 0x00);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_6 |GPIO_PIN_7, 0x00);

	/*RS =1*/
	GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6, 0x40);

	/*Put command on data lines */
	GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0 |GPIO_PIN_1|GPIO_PIN_2 |GPIO_PIN_3, data);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_6 |GPIO_PIN_7, data);

	/*Generate a high to low pulse on enable */
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0x01);
	SysCtlDelay(1340);
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0x00);

}


void glcd_setpage (unsigned char page)
{
	/*set CS1(CS1=1 and CS2=0) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x00);

	/*Select the page*/
	glcd_cmd(0xB8 | page);
	SysCtlDelay(10);

	/*set CS2(CS1=0 and CS2=1) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x08);

	/*select the page*/
	glcd_cmd(0xB8 | page);
	SysCtlDelay(10);

}

void glcd_setcolumn(unsigned char column)
{

	if(column < 64)
	{
		/* set CS1(CS1=1 and CS2=0) */
		GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x00);

		/* Select column on left side */
		glcd_cmd(0x40 | column);
		SysCtlDelay(10);
	}
	else	//	right side is selected for column>64
	{
		GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x08);

		glcd_cmd(0x40 | (column-64) );
		SysCtlDelay(10);

	}
}

void glcd_cleardisplay()
{
	unsigned char i,j;
	for(i=0;i<8;i++)
	{
		glcd_setpage(i);
		for(j=0;j<128;j++)
		{
			glcd_setcolumn(j);
			glcd_data(0x00);

		}
	}
}


int main(void)
{
	int j, img=0;
	unsigned char i,p;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE+GPIO_O_CR) |= GPIO_PIN_0;

	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_6 |GPIO_PIN_7);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6 );
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0 |GPIO_PIN_1|GPIO_PIN_2 |GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 );


	glcd_init();
	glcd_cleardisplay();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ADCHardwareOversampleConfigure(ADC1_BASE, 64);

	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

	ADCSequenceConfigure(ADC1_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);

	ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC1_BASE,1,3,ADC_CTL_CH6|ADC_CTL_IE|ADC_CTL_END);

	ADCSequenceEnable(ADC0_BASE, 1);
	ADCSequenceEnable(ADC1_BASE, 1);


	while(1)
	{

		ADCIntClear(ADC0_BASE, 1);
		ADCIntClear(ADC1_BASE, 1);

		ADCProcessorTrigger(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC1_BASE, 1);

		while(!ADCIntStatus(ADC0_BASE, 1, false) && !ADCIntStatus(ADC1_BASE, 1, false))
		{
		}

		ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
		ADCSequenceDataGet(ADC1_BASE, 1, ui32ADC1Value);
		avg = (ui32ADC1Value[0] + ui32ADC1Value[1] + ui32ADC1Value[2] + ui32ADC1Value[3] + 2)/4;

		x= avg;
		delay = 100000;
		if(x < 200)
		{
			delay= delay * 100;
		}

		if(x > 3900)
		{
			delay= delay/1000;
		}

		glcd_cleardisplay();

		j=0;
		p=0;
		while(p < 8)
		{
			glcd_setpage(p);

			for(i=0;i<128;i++)
			{

				glcd_setcolumn(i);

				if (img == 0)
				{
					glcd_data(one[j]);
					j++;
				}
				else if (img == 1)
				{
					glcd_data(two[j]);
					j++;
				}
				else if (img == 2)
				{
					glcd_data(three[j]);
					j++;
				}
				else if (img == 3)
				{
					glcd_data(four[j]);
					j++;
				}
				else if (img == 4)
				{
					glcd_data(five[j]);
					j++;
				}
				else if (img == 5)
				{
					glcd_data(six[j]);
					j++;
				}
				else if (img == 6)
				{
					glcd_data(seven[j]);
					j++;
				}
				else if (img == 7)
				{
					glcd_data(eight[j]);
					j++;
				}

			}

			p++;
		}
		SysCtlDelay(delay);
		img++;
		img = img % 8;
	}
	return 0;
}
