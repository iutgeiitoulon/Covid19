#include <p33FJ128MC804.h>
#include "adc.h"

unsigned char ADCResultIndex=0;
static unsigned int ADCResult[4];
unsigned char ADCConversionFinishedFlag;

/****************************************************************************************************/
// Configuration ADC
/****************************************************************************************************/
void InitADC1(void)
{
    //Configuration en mode 12 bits mono canal ADC avec conversions successives sur 4 entrées
    /************************************************************/
    //AD1CON1
    /************************************************************/
    AD1CON1bits.ADON = 0;       // ADC module OFF - pendant la config
    AD1CON1bits.AD12B = 1;      // 0 : 10bits - 1 : 12bits
    AD1CON1bits.FORM = 0b00;    // 00 = Integer (DOUT = 0000 dddd dddd dddd)
    AD1CON1bits.ASAM = 0;       // 0 = Sampling begins when SAMP bit is set
    AD1CON1bits.SSRC = 0b111;   // 111 = Internal counter ends sampling and starts conversion (auto-convert)

    /************************************************************/
    //AD1CON2
    /************************************************************/
    AD1CON2bits.VCFG = 0b000;   // 000 : Voltage Reference = AVDD AVss
    AD1CON2bits.CSCNA = 1;      // 1 : Enable Channel Scanning
    AD1CON2bits.SMPI = 3;       // 3+1 conversions successives (une interruption à chaque fois)

    /************************************************************/
    //AD1CON3
    /************************************************************/
    AD1CON3bits.ADRC = 0;       // ADC Clock is derived from Systems Clock
    AD1CON3bits.SAMC = 0;       // Auto Sample Time = 0 * TAD
    AD1CON3bits.ADCS = 30;       // ADC Conversion Clock TAD = TCY * (ADCS + 1)
                                // = (1/16MHz) * 2 = 125ns
                                // 75 ns est le minimum pour TAD
                                // ADC Conversion Time for 12-bit Tconv = 14 * TAD
                                // = 1750 ns (0.6 MHz)

    /************************************************************/
    //Configuration des ports
    /************************************************************/
    AD1PCFGLbits.PCFG4 = 0; // AN4 as Analog Input
    AD1PCFGLbits.PCFG5 = 0; // AN5 as Analog Input
    AD1PCFGLbits.PCFG6 = 0; // AN6 as Analog Input
    AD1PCFGLbits.PCFG7 = 0; // AN7 as Analog Input

    AD1CSSLbits.CSS4=1; // Enable AN4 for scan
    AD1CSSLbits.CSS5=1; // Enable AN5 for scan
    AD1CSSLbits.CSS6=1; // Enable AN6 for scan
    AD1CSSLbits.CSS7=1; // Enable AN7 for scan

    IFS0bits.AD1IF = 0; // Clear the A/D interrupt flag bit
    IEC0bits.AD1IE = 1; // Enable A/D interrupt
    AD1CON1bits.ADON = 1; // Turn on the A/D converter
}


/* This is ADC interrupt routine */
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt (void)
{
	IFS0bits.AD1IF = 0;
        ADCResult[ADCResultIndex] = ADC1BUF0;
        if( ADCResultIndex < 3)
            ADCResultIndex++;
        else
        {
            ADCResultIndex=0;
            ADCConversionFinishedFlag = 1;
        }
}

void ADC1StartConversionSequence()
{
    AD1CON1bits.SAMP = 1 ; //Lance une acquisition ADC
}

unsigned int * ADCGetResult(void)
{
    return ADCResult;
}

unsigned char ADCIsConversionFinished(void)
{
    return ADCConversionFinishedFlag;
}

void ADCClearConversionFinishedFlag(void)
{
    ADCConversionFinishedFlag = 0;
}
