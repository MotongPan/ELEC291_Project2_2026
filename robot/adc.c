#include  "../Common/Include/stm32l051xx.h"

// All of this code is mostly copy/paste from the STM32L05X reference manual RM0451.

void initADC(void)
{
	RCC->APB2ENR |= BIT9;
	ADC1->CFGR2 |= ADC_CFGR2_CKMODE;
	ADC1->ISR |= ADC_ISR_ADRDY; /* (1) */
	ADC1->CR |= ADC_CR_ADEN; /* (2) */
	if ((ADC1->CFGR1 & ADC_CFGR1_AUTOFF) == 0)
	{
		while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) /* (3) */
		{
			/* For robust implementation, add here time-out management */
		}
	}	

	if ((ADC1->CR & ADC_CR_ADEN) != 0) /* (1) */
	{
		ADC1->CR |= ADC_CR_ADDIS; /* (2) */
	}
	ADC1->CR |= ADC_CR_ADCAL; /* (3) */
	while ((ADC1->ISR & ADC_ISR_EOCAL) == 0) /* (4) */
	{
		/* For robust implementation, add here time-out management */
	}
	ADC1->ISR |= ADC_ISR_EOCAL; /* (5) */
}

int readADC(unsigned int channel)
{
	ADC1->CFGR1 |= ADC_CFGR1_AUTOFF; /* (2) */
	ADC1->CHSELR = channel; /* (3) */
	ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2; /* (4) */
	if(channel==ADC_CHSELR_CHSEL17)
	{
		ADC->CCR |= ADC_CCR_VREFEN; /* (5) */
	}
	
	/* Performs the AD conversion */
	ADC1->CR |= ADC_CR_ADSTART; /* start the ADC conversion */
	while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* wait end of conversion */
	{
		/* For robust implementation, add here time-out management */
	}

	return ADC1->DR; // ADC_DR has the 12 bits out of the ADC
}
unsigned int adc_read_left(void)
{
    return readADC(ADC_CHSELR_CHSEL0);
}

unsigned int adc_read_right(void)
{
    return readADC(ADC_CHSELR_CHSEL1);
}

unsigned int adc_read_intersection(void)
{
    return readADC(ADC_CHSELR_CHSEL4);
}
void ADC_Pin_Init(void)
{
    RCC->IOPENR |= BIT0; // GPIOA clock enable

    // PA0 -> analog
    GPIOA->MODER |= (BIT0 | BIT1);

    // PA1 -> analog
    GPIOA->MODER |= (BIT2 | BIT3);

    // PA4 -> analog
    GPIOA->MODER |= (BIT8 | BIT9);
}
