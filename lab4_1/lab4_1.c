#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"

	uint32_t var[4];	// sequencer buffer
	uint32_t hor[4];

	// getting temp from sensor data and is not optimized by compiler
	//volatile uint32_t ui32TempAvg;
	//volatile uint32_t ui32TempValueC;
	//volatile uint32_t ui32TempValueF;
	volatile uint32_t horAvg;
	volatile uint32_t verAvg;


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
	}
}
