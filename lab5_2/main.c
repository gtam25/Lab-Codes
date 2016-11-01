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

void glcd_init();
void glcd_cleardisplay();
void glcd_data(unsigned char );
void glcd_setcolumn(unsigned char );
void glcd_setpage (unsigned char );
void glcd_cmd(unsigned char);


uint32_t ui32ADC0Value[4]; //create an array that will be used for storing the data read from the ADC FIFO
volatile uint32_t avg1,avg2;
//volatile uint32_t ui32Value1,ui32Value2;
uint32_t ui32ADC1Value[4];
volatile uint32_t x, y;


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
	SysCtlDelay(50);

	/*set CS2(CS1=0 and CS2=1) */
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x08);

	/*select the page*/
	glcd_cmd(0xB8 | page);
	SysCtlDelay(50);

}

void glcd_setcolumn(unsigned char column)
{

	if(column < 64)
	{
		/* set CS1(CS1=1 and CS2=0) */
		GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x00);

		/* Select column on left side */
		glcd_cmd(0x40 | column);
		SysCtlDelay(6700);
	}
	else	//	right side is selected for column>64
	{
		GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_3, 0x08);

		glcd_cmd(0x40 | (column-64) );
		SysCtlDelay(6700);

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

	unsigned char i;
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

	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);//enable ADC0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);//enable ADC1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);	// ADC
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ADCHardwareOversampleConfigure(ADC1_BASE, 64);

	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);//configure the ADC sequencer. We want to use ADC0, sample sequencer 1,
	//we want the processor to trigger the sequence and we want to use the highest priority.
	ADCSequenceConfigure(ADC1_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);

	ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC1_BASE,1,3,ADC_CTL_CH6|ADC_CTL_IE|ADC_CTL_END);

	ADCSequenceEnable(ADC0_BASE, 1); //enable ADC sequencer 1 of ADC0
	ADCSequenceEnable(ADC1_BASE, 1); //enable ADC sequencer 1 of ADC1


	while(1)
	{
		ADCIntClear(ADC0_BASE, 1);//ADC interrupt status flag
		ADCIntClear(ADC1_BASE, 1);

		ADCProcessorTrigger(ADC0_BASE, 1); //trigger the ADC conversion with software
		ADCProcessorTrigger(ADC1_BASE, 1);

		while(!ADCIntStatus(ADC0_BASE, 1, false) && !ADCIntStatus(ADC1_BASE, 1, false))
		{
		}

		ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);//read the ADC value from the ADC Sample Sequencer 1 FIFO
		ADCSequenceDataGet(ADC1_BASE, 1, ui32ADC1Value);//read the ADC value from the ADC Sample Sequencer 1 FIFO
		avg1 = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
		avg2 = (ui32ADC1Value[0] + ui32ADC1Value[1] + ui32ADC1Value[2] + ui32ADC1Value[3] + 2)/4;
		x = 127 -(avg2/32);
		y = 7 - (avg1/512);


		if(x >=128)
		{
			x=127;
		}

		if(y >=8)
		{
			x=7;
		}


		//displaying contents on glcd

		glcd_cleardisplay();
			//set the page

			glcd_setpage(y);

			for(i=x-4;i<=x+4;i++)
			{

				glcd_setcolumn(i);

				//send hex value of data to GLCD
				glcd_data(0xFF);


			}


	}

	return 0;
}
