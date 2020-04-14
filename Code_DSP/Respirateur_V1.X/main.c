#include <stdio.h>
#include <stdlib.h>
#include "p33Fxxxx.h"
#include <libpic30.h>
#include "pragma_config.h"
#include "main.h"
#include "oscillator.h"
#include "IO.h"
#include "ADC.h"
#include "UTLN_Typedefs.h"
#include "UTLN_Timers.h"
#include "PWM.h"
#include "OutputCompare.h"
#include "UTLN_uart.h"
#include "UTLN_Communication.h"
#include "RespiratorState.h"
#include "Define.h"

#define SENS_MONTEE 1
#define SENS_DESCENTE 0
volatile unsigned long g_longTimeStamp=0;

extern BOOL startRespirator;
void Timer1CallBack(void);
void Timer2CallBack(void);
void Timer3CallBack(void);
void Timer4CallBack(void);
void Timer5CallBack(void);

typedef enum{
    ARRET,
    ATTENTE_BAS,
    MONTEE,
    DESCENTE,
    ATTENTE_HAUT
}ETAT;



ETAT etat=ARRET;
volatile unsigned long timeStampDebut=0;
unsigned char flagFinMontee=0;
unsigned char flagFinDescente=0;
int main(void)
{
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entr�es sorties
    /****************************************************************************************************/
    InitIO();
    InitCN();           //Initialisation des pullUP (Change Notification)
    
    InitADC1();
    
    RegisterTimerWithCallBack(TIMER2_ID, 10000.0, Timer2CallBack, true, 3, 1);   //Time base pulse OC
    RegisterTimerWithCallBack(TIMER3_ID, 500.0, Timer3CallBack, true, 5, 1);//Gestion de la vitesse des pas a pas
    RegisterTimerWithCallBack(TIMER4_ID, 1000.0, Timer4CallBack, true, 6, 1);//TimeStamp
    RegisterTimerWithCallBack(TIMER5_ID, 50.0, Timer5CallBack, true, 4, 1);//Timer Send values
    InitOC1();
    initUART1();
    
    LED_BLANCHE = 1;
    LED_ROUGE = 1;
    
    //Parametres par defaut
    respiratorState.amplitude=475;
    respiratorState.attenteBas=1500;
    respiratorState.attenteHaut=1500;
    respiratorState.stepsOffsetDown=0;
    respiratorState.stepsOffsetUp=0;
    respiratorState.cpt=0;
    respiratorState.useExternalPotentiometre=0;
    respiratorState.vitesse=1300;
    
    //STEP=1;
    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    for (;;)
    {
        //Detection fin course
        if(FIN_COURSE1==0)
        {
             respiratorState.cpt=0;  //On reset le compteur(et donc la position 0 du moteur)
        }
        if(ADCIsConversionFinished())
        {
            ADCClearConversionFinishedFlag();
            unsigned int ADCVals[5];
            unsigned int * pValues=ADCGetResult();
            ADCVals[0]=pValues[0];
            ADCVals[1]=pValues[1];
            ADCVals[2]=pValues[2];
            ADCVals[3]=pValues[3];
            ADCVals[4]=pValues[4];
            respiratorState.pressure1=(ADCVals[0]*(3.3/4096));
            respiratorState.pressure2=(ADCVals[1]*(3.3/4096));
            if(respiratorState.useExternalPotentiometre)
            {
                respiratorState.amplitude=(unsigned short)(ADCVals[2]*(500.0/4096.0));
                respiratorState.vitesse=(ADCVals[3]*(1300.0/4096));
                SetTimerFreq(TIMER3_ID,respiratorState.vitesse);
            }
        }
        
        switch(etat)
        {
            case ARRET:
                if(startRespirator)
                {
                    etat=ATTENTE_BAS;
                    timeStampDebut=g_longTimeStamp;
                }
                break;
            case ATTENTE_BAS:
                if(g_longTimeStamp>=(timeStampDebut+respiratorState.attenteBas))
                {
                    //timeStampDebut=g_longTimeStamp;
                    LED_ROUGE=1;
                    //On commence la montee
                    respiratorState.sens=SENS_MONTEE;
                    DIR=SENS_MONTEE;
                    TurnOnOffTimer(TIMER3_ID,1);
                    
                    etat=MONTEE;
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            case MONTEE:
                if(flagFinMontee)
                {
                    LED_ROUGE=0;
                    flagFinMontee=0;
                    timeStampDebut=g_longTimeStamp;
                    etat=ATTENTE_HAUT;
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            case ATTENTE_HAUT:
                if(g_longTimeStamp>=(timeStampDebut+respiratorState.attenteHaut))
                {
                    //On commence la descente
                    respiratorState.sens=SENS_DESCENTE;
                    DIR=SENS_DESCENTE;
                    TurnOnOffTimer(TIMER3_ID,1);
                    
                    etat=DESCENTE;
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            case DESCENTE:
                if(flagFinDescente)
                {
                    flagFinDescente=0;
                    timeStampDebut=g_longTimeStamp;
                    etat=ATTENTE_BAS;
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            default:break;
        }
    } 
}// fin main



void Timer2CallBack(void)
{
    
}
            


void Timer3CallBack(void)
{
    LED_BLANCHE=!LED_BLANCHE;
    if(respiratorState.sens==SENS_MONTEE)
    {
        //Si on es dans le sens positif
        OC1GeneratePulse(); //On genere un pulse
        respiratorState.cpt++;              //On incremente le compteur
        
        if(respiratorState.cpt>=respiratorState.amplitude)
        {
            flagFinMontee=1;
            TurnOnOffTimer(TIMER3_ID,OFF);      //On arrete le timer
            //Si on a atteint l'amplitude Max, on change le sens
           // DIR=!DIR;
           // sens=0;
        }
    }
    else
    {
        //Si on es dans le sens negatif
        OC1GeneratePulse(); //On genere un pulse
        respiratorState.cpt--;              //On decremente le compteur
        
        if(respiratorState.cpt<=0)
        {
            flagFinDescente=1;
            TurnOnOffTimer(TIMER3_ID,OFF);      //On arrete le timer
            //Si on a atteint l'amplitude Min, on change le sens
           // DIR=!DIR;
           // sens=1;
        }
    }
}
void Timer4CallBack(void)
{
    g_longTimeStamp++;
    ADC1StartConversionSequence();
}
void Timer5CallBack(void)
{
    unsigned char payload[16];
    payload[0]=BREAK_UINT32(g_longTimeStamp,3);
    payload[1]=BREAK_UINT32(g_longTimeStamp,2);
    payload[2]=BREAK_UINT32(g_longTimeStamp,1);
    payload[3]=BREAK_UINT32(g_longTimeStamp,0);
    getBytesFromFloat(payload, 4, respiratorState.pressure1);
    getBytesFromFloat(payload, 8, respiratorState.pressure2);
    getBytesFromFloat(payload, 12, (float)respiratorState.cpt);
    MakeAndSendMessageWithUTLNProtocol(0x0064, 16, payload);
}
void SetMoteurPAPSpeed(float speed)
{
    
}