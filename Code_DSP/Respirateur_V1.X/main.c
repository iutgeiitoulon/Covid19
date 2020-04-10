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

#define SENS_MONTEE 1
#define SENS_DESCENTE 0
volatile unsigned long g_longTimeStamp=0;


void Timer4CallBack(void);
void Timer2CallBack(void);
void Timer3CallBack(void);

typedef enum{
    ARRET,
    ATTENTE_BAS,
    MONTEE,
    DESCENTE,
    ATTENTE_HAUT
}ETAT;



ETAT etat=ATTENTE_BAS;
volatile unsigned long timeStampDebut=0;
unsigned char flagFinMontee=0;
unsigned char flagFinDescente=0;
double periodeRespi=2.0;        //Periode en seconde
volatile unsigned long attenteHaut=1000;         //Attente haut en miliseconde
volatile unsigned long attenteBas=1000;         //Attente bas en miliseconde
double amplitudeMax=1000;           //474 pas
double vitesse=1300;
double amplitudeMin=0;
unsigned char sens=0;
double cpt=0;
int main(void)
{
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entrées sorties
    /****************************************************************************************************/
    InitIO();
    InitCN();           //Initialisation des pullUP (Change Notification)
    
    InitADC1();
    
    RegisterTimerWithCallBack(TIMER2_ID, 10000.0, Timer2CallBack, true, 3, 1);   //Time base pulse OC
    RegisterTimerWithCallBack(TIMER3_ID, 500.0, Timer3CallBack, true, 5, 1);//Gestion de la vitesse des pas a pas
    RegisterTimerWithCallBack(TIMER4_ID, 1000.0, Timer4CallBack, true, 6, 1);//TimeStamp
    InitOC1();
    LED_BLANCHE = 1;
    LED_ROUGE = 1;
    //STEP=1;
    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    for (;;)
    {
        //Detection fin course
        if(FIN_COURSE1==0)
        {
             cpt=0;  //On reset le compteur(et donc la position 0 du moteur)
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
            amplitudeMax=(ADCVals[2]*(500.0/4096.0));
            vitesse=(ADCVals[3]*(1300.0/4096));
            SetTimerFreq(TIMER3_ID,vitesse);
        }
        switch(etat)
        {
            case ARRET: break;
            case ATTENTE_BAS:
                if(g_longTimeStamp>=(timeStampDebut+attenteBas))
                {
                    timeStampDebut=g_longTimeStamp;
                    LED_ROUGE=1;
                    //On commence la montee
                    sens=SENS_MONTEE;
                    DIR=SENS_MONTEE;
                    TurnOnOffTimer(TIMER3_ID,1);
                    
                    etat=MONTEE;
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
                break;
            case ATTENTE_HAUT:
                if(g_longTimeStamp>=(timeStampDebut+attenteHaut))
                {
                    //On commence la descente
                    sens=SENS_DESCENTE;
                    DIR=SENS_DESCENTE;
                    TurnOnOffTimer(TIMER3_ID,1);
                    
                    etat=DESCENTE;
                }
                break;
            case DESCENTE:
                if(flagFinDescente)
                {
                    flagFinDescente=0;
                    timeStampDebut=g_longTimeStamp;
                    etat=ATTENTE_BAS;
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
    if(sens==SENS_MONTEE)
    {
        //Si on es dans le sens positif
        OC1GeneratePulse(); //On genere un pulse
        cpt++;              //On incremente le compteur
        
        if(cpt>=amplitudeMax)
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
        cpt--;              //On decremente le compteur
        
        if(cpt<=amplitudeMin)
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
void SetMoteurPAPSpeed(float speed)
{
    
}