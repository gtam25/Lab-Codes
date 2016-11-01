#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

uint32_t var[4];	// sequencer buffer
uint32_t hor[4];
volatile uint32_t horAvg;
volatile uint32_t verAvg;
char x,y;

int main(void) {


	// system clock at 40 MHz
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

	// enable the ADC0 peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	// configure ADC0 sequencer1 and processor trigers the sequence with highest priority
	// ADC12 to run at its default rate of 1Msps.
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);

	// sample the temperature sensor
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH7);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH7);
	// after the last sample configure the interrupt flag and Tell the ADC logic that this is the last conversion on sequencer 1
	ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);

	ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH6);
	ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_CH6|ADC_CTL_IE|ADC_CTL_END);


	// enable sequencer 1
	ADCSequenceEnable(ADC0_BASE, 1);
	ADCSequenceEnable(ADC0_BASE, 2);


	// UART
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); //enable pin for LED PF2
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	IntMasterEnable(); //enable processor interrupts
	IntEnable(INT_UART0); //enable the UART interrupt
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts

	while(1)
	{
		// clear ADC interrupt status flag
		ADCIntClear(ADC0_BASE, 1);
		ADCIntClear(ADC0_BASE, 2);

		// trigger the ADC conversion (with software)
		ADCProcessorTrigger(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC0_BASE, 2);

		// wait for conversion to complete
		while(!ADCIntStatus(ADC0_BASE, 1, false))
		{
		}

		while(!ADCIntStatus(ADC0_BASE, 2, false))
		{
		}

		// copy data from sequencer to our buffer
		ADCSequenceDataGet(ADC0_BASE, 1, var);
		ADCSequenceDataGet(ADC0_BASE, 2, hor);

		// cal the avg
		verAvg = (var[0] + var[1] + var[2] + var[3] + 2)/4;
		horAvg = (hor[0] + hor[1] + hor[2] + hor[3] + 2)/4;

		// convert to celsius
		//TEMP = 147.5 – ((75 * (VREFP – VREFN) * ADCVALUE) / 4096)
		//ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;

		// cal the Fahrenheit
		//ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

		UARTCharPut(UART0_BASE, 'X');
		UARTCharPut(UART0_BASE, ':');

		x =  (char)(horAvg/1000);
		UARTCharPut(UART0_BASE, x+48);
		horAvg = horAvg%1000;
		x = (char)(horAvg/100);
		UARTCharPut(UART0_BASE, x+48);
		horAvg = horAvg%100;
		x = (char)(horAvg/10);
		UARTCharPut(UART0_BASE, x+48);
		horAvg = horAvg%10;
		x = (char)horAvg;
		UARTCharPut(UART0_BASE, x+48);

		UARTCharPut(UART0_BASE, '\n');

		UARTCharPut(UART0_BASE, 'Y');
		UARTCharPut(UART0_BASE, ':');

		y = (char)(verAvg/1000);
		UARTCharPut(UART0_BASE, y+48);
		verAvg = verAvg%1000;
		y = (char)(verAvg/100);
		UARTCharPut(UART0_BASE, y+48);
		verAvg = verAvg%100;
		y = (char)(verAvg/10);
		UARTCharPut(UART0_BASE, y+48);
		verAvg = verAvg%10;
		y = (char)verAvg;
		UARTCharPut(UART0_BASE, y+48);

		UARTCharPut(UART0_BASE, '\n');

		SysCtlDelay(5000000);
	}
}

/*
void UARTIntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts
	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE)); //echo character
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
		SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
	}
}

	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, 'n');
	UARTCharPut(UART0_BASE, 't');
	UARTCharPut(UART0_BASE, 'e');
	UARTCharPut(UART0_BASE, 'r');
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, 'e');
	UARTCharPut(UART0_BASE, 'x');
	UARTCharPut(UART0_BASE, 't');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
	while (1) //let interrupt handler do the UART echo function
	 {
		 if (UARTCharsAvail(UART0_BASE))
		 {
			 //UARTCharPut(UART0_BASE, UARTCharGet(UART0_BASE));

		 }
	 }
}
*/
