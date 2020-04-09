/* 
 * File:   ADC.h
 * Author: Valentin
 *
 * Created on 4 octobre 2015, 09:34
 */

#ifndef ADC_H
#define	ADC_H

void InitADC1(void);
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt (void);
void ADC1StartConversionSequence();
unsigned int * ADCGetResult(void);
unsigned char ADCIsConversionFinished(void);
void ADCClearConversionFinishedFlag(void);

#endif	/* ADC_H */

