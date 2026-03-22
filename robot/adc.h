#ifndef __ADC_H__
#define __ADC_H__

void ADC_Pin_Init(void);
void initADC(void);
int readADC(unsigned int channel);

unsigned int adc_read_left(void);
unsigned int adc_read_right(void);
unsigned int adc_read_intersection(void);

#endif
