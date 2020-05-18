#include <stdio.h>
#include <stdlib.h>
#include "p33Fxxxx.h"
#include <libpic30.h>
#include <math.h>
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
#include "ustv_i2c_interrupt.h"
#include "UTLN_D6F-PH.h"

#define SENS_MONTEE 0
#define SENS_DESCENTE 1
volatile unsigned long g_longTimeStamp=0;

BOOL boardVersion3Moteur=true;
extern BOOL startRespirator;

void StateMachineMoteur1(void);
void StateMachineMoteur2(void);
void StateMachineMoteur3(void);

void Timer1CallBack(void);
void Timer2CallBack(void);
void Timer3CallBack(void);
void Timer4CallBack(void);
void Timer5CallBack(void);
void CalculateRespiratorParameters(void);

double surface=0;
typedef enum{
    ARRET,
    INIT_0,
    MONTEE,        
    ATTENTE_BAS,
    DESCENTE,
    ATTENTE_HAUT,
    VENTILATION,
    MANUAL
}ETAT;



volatile ETAT etat=ARRET;
volatile unsigned long timeStampDebut=0;
volatile unsigned long timeStampHighSpeed=0;
unsigned char flagFinMontee=0;
unsigned char flagFinDescente=0;
unsigned long deadTime=100000;
int main(void)
{
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entrées sorties
    /****************************************************************************************************/
    InitIO();
    InitCN();           //Initialisation des pullUP (Change Notification)
    
    //InitADC1();
    
    RegisterTimerWithCallBack(TIMER2_ID, 1000000.0, Timer2CallBack, true, 3, 1);   //Time base pulse OC
    RegisterTimerWithCallBack(TIMER3_ID, 1200.0, Timer3CallBack, true, 5, 0);//Gestion de la vitesse des pas a pas
    RegisterTimerWithCallBack(TIMER4_ID, 1000.0, Timer4CallBack, true, 6, 1);//TimeStamp
    RegisterTimerWithCallBack(TIMER5_ID, 50.0, Timer5CallBack, true, 4, 1);//Timer Send values
    InitOC1();
    InitOC2();
    InitOC3();
    initUART1();
                                      
    InitI2C1();
    //D6FHarwareReset();
    __delay_ms(10);
    //D6F_PHInitialize();
    __delay_ms(100);
    LED_BLANCHE = 1;
    LED_ROUGE = 1;
    nRESET=0;       //Desactivation moteurs
    //Parametres par defaut
    respiratorState.amplitude=600;
    respiratorState.attenteBas=1000;
    respiratorState.attenteHaut=1000;
    respiratorState.stepsOffsetDown=0;
    respiratorState.stepsOffsetUp=0;
    respiratorState.cptMoteur1=0;
    respiratorState.cptMoteur2=0;
    respiratorState.cptMoteur3=0;
    respiratorState.vitesse=1300;
    respiratorState.pLimite=2500;      //en Pascals
    respiratorState.vLimite=0.5;     //en L
    respiratorState.periode=2000000;    //(en us)
    respiratorState.cyclesPerMinute=12;
    
    /****************************************************************************************************/
    // Determination du type de respirateur (1 moteur VS 3 Moteurs)
    /****************************************************************************************************/
    if(FIN_COURSE5==0 && FIN_COURSE6==0)
    {
        boardVersion3Moteur=false;
        surface=7.26/100000;
        CalculateRespiratorParameters();
    }
    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    for (;;)
    {
        //Detection fin course
        if(FIN_COURSE1==0)
        {
             respiratorState.cptMoteur1=0;  //On reset le compteur(et donc la position 0 du moteur)
             LED_BLANCHE=1;
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
            //respiratorState.pressure1=(ADCVals[0]*(3.3/4096));
            respiratorState.pressure2=(ADCVals[0]*(3.3/4096));
            
            double rho = 1.23;
            double diametre = 0.023;        //en M
            double diffPression = respiratorState.pressure2-0.08;
            int sign=1;
            if (diffPression < 0)
                sign = -1;
            else
                sign = 1;
            double vitesse=sqrt(2*ABS(diffPression)/rho)*sign;
            respiratorState.debitCourant=vitesse*0.0009;
        }
        
        switch(etat)
        {
            case ARRET:
                if(startRespirator)
                {
                    nRESET=1;
                    if(boardVersion3Moteur)
                    {
                        etat=INIT_0;
                    }
                    else
                    {
                        etat=ATTENTE_BAS;
                        RegisterTimerWithCallBack(TIMER3_ID, 500.0, Timer3CallBack, true, 5, 0);//Gestion de la vitesse des pas a pas
                    }
                    timeStampDebut=g_longTimeStamp;
                }
                else
                {
                    nRESET=0;       //Desactivation moteurs
                }
                if(respiratorState.flagDoStepsCMD)
                {
                    etat=MANUAL;
                    TurnOnOffTimer(TIMER3_ID,1);
                    nRESET=1;
                }
                break;
            case INIT_0:
                TurnOnOffTimer(TIMER3_ID,1);
                etat=VENTILATION;
                break;
            case ATTENTE_BAS:
                if(g_longTimeStamp>=(timeStampDebut+(unsigned long)respiratorState.attenteBas))
                {
                    //timeStampDebut=g_longTimeStamp;
                    LED_ROUGE=1;
                    //On commence la montee
                    respiratorState.sensMoteur1=SENS_MONTEE;
                    DIR1=SENS_MONTEE;
                    TurnOnOffTimer(TIMER3_ID,1);
                    respiratorState.volumeCourant=0;
                    etat=MONTEE;
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            //Montee en pression    
            case MONTEE:
                if(flagFinMontee)
                {
                    LED_BLANCHE=0;
                    LED_ROUGE=0;
                    flagFinMontee=0;
                    timeStampDebut=g_longTimeStamp;
                    etat=ATTENTE_HAUT;
                }
                if(respiratorState.pressure2>=respiratorState.pLimite || respiratorState.volumeCourant>=respiratorState.vLimite)
                {
                    timeStampDebut=g_longTimeStamp;
                    etat=ATTENTE_HAUT;
                    TurnOnOffTimer(TIMER3_ID,0);
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            case ATTENTE_HAUT:
                if(g_longTimeStamp>=(timeStampDebut+(unsigned long)respiratorState.attenteHaut))
                {
                    //On commence la descente
                    respiratorState.sensMoteur1=SENS_DESCENTE;
                    DIR1=SENS_DESCENTE;
                    TurnOnOffTimer(TIMER3_ID,1);
                    timeStampDebut=g_longTimeStamp;
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
                    
                    etat=ATTENTE_BAS;
                }
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;          
            case VENTILATION:
                if(!startRespirator)
                {
                    TurnOnOffTimer(TIMER3_ID,0);
                    etat=ARRET;
                }
                break;
            case MANUAL:
                if(respiratorState.flagDoStepsCMD==0)
                {
                    etat=ARRET;
                }
                break;
            
            default:break;
        }
    } 
}// fin main

//<editor-fold defaultstate="collapsed" desc="State Machines">
void StateMachineMoteur1(void)
{
    static unsigned char etat1=0;
    static unsigned int cpt1=0;
    switch(etat1)
    {
        //Calage 0 (bas)
        case 0:DIR1=SENS_DESCENTE;
            OC1GeneratePulse();
            if(FIN_COURSE1==0)
            {
                etat1=1;
            }
            break;
        case 1:timeStampHighSpeed=0;
            DIR1=SENS_DESCENTE;
            etat1=2;
            break;
        //DESCENTE    
        case 2:
            cpt1++;
            if(cpt1==3)
            {
                OC1GeneratePulse(); //On genere un pulse
                cpt1=1;
            }
            if(timeStampHighSpeed> (2*respiratorState.periode/3) || FIN_COURSE1==0)
            {
                etat1=3;
                DIR1=SENS_MONTEE;
            }
            break;
        //Attente    
        case 3:if(timeStampHighSpeed>=2*respiratorState.periode/3+deadTime)
                {
                    etat1=4;
                }
            break;
        //MONTEE    
        case 4:
            OC1GeneratePulse(); //On genere un pulse
            if( FIN_COURSE2==0 || timeStampHighSpeed>=respiratorState.periode-deadTime)
            {
                etat1=5;
            }
            break;
        //ATTENTE    
        case 5:
            if(timeStampHighSpeed>=respiratorState.periode)
            {
                etat1=1;
            }
            break;
    }
}

void StateMachineMoteur2(void)
{
    static unsigned char etat2=0;
    static unsigned int cpt2=0;
    
    //Descente entre T/3 et T
    //DeadTime Bas entre 0 et deadTime
    //Montee entre deadTime et T/3-deadTime
    //DeadTime Haut entre T/3-deadTime et T/3
    switch(etat2)
    {
        //Calage 0 (bas)
        case 0:DIR1=SENS_DESCENTE;
            OC2GeneratePulse();
            if(FIN_COURSE3==0)
            {
                etat2=1;
            }
            break;
        case 1:
            DIR2=SENS_DESCENTE;
            etat2=2;
            break;
        //DESCENTE    
        case 2:
        {
            LED_BLANCHE=0;
            LED_ROUGE =1;
            
            cpt2++;
            if(cpt2==3)
            {
                OC2GeneratePulse(); //On genere un pulse
                cpt2=1;
            }
            
            //On teste si on est toujours en mode descente d'après le timestamp
            int descenteActive = 0;
            if(timeStampHighSpeed >= (respiratorState.periode/3) && timeStampHighSpeed < respiratorState.periode)
                descenteActive = 1;
                     
            if(descenteActive == 0 || FIN_COURSE3==0)
            {
                etat2=3;
                DIR2=SENS_MONTEE;
            }
            break;
        }
        //Dead Time Bas
        case 3:            
            LED_BLANCHE=0;
            LED_ROUGE =0;
            if(timeStampHighSpeed>=deadTime && timeStampHighSpeed<respiratorState.periode/3)
            {
                etat2=4;
            }
            break;            
        //MONTEE    
        case 4:
        {
            LED_BLANCHE=1;
            LED_ROUGE =0;
            OC2GeneratePulse(); //On genere un pulse
            
            //On teste si on est toujours en mode descente d'après le timestamp
            int monteeActive = 0;
            if(timeStampHighSpeed >= deadTime && timeStampHighSpeed < respiratorState.periode/3-deadTime)
                monteeActive = 1;
            
            if(monteeActive==0 || FIN_COURSE4==0)
            {
                etat2=5;
            }
            break;
        }
        //ATTENTE    
        case 5:
            LED_BLANCHE=0;
            LED_ROUGE =0;
            if(timeStampHighSpeed>=respiratorState.periode/3)
            {
                etat2=1;
            }
            break;
    }
}

void StateMachineMoteur3(void)
{
    static unsigned char etat3=0;
    static unsigned int cpt3=0;
    switch(etat3)
    {
        //Calage 0 (bas)
        case 0:DIR1=SENS_DESCENTE;
            OC3GeneratePulse();
            if(FIN_COURSE5==0)
            {
                etat3=1;
            }
            break;        
        case 1:
            DIR3=SENS_DESCENTE;
            etat3=2;
            break;
        //DESCENTE    
        case 2:
            cpt3++;
            if(cpt3==3)
            {
                OC3GeneratePulse(); //On genere un pulse
                cpt3=1;
            }
            if((timeStampHighSpeed>=(respiratorState.periode/3) && timeStampHighSpeed<(2*respiratorState.periode/3)) || FIN_COURSE5==0)
            {
                etat3=3;
                DIR3=SENS_MONTEE;
            }
            break;
        //Attente    
        case 3:
            if((timeStampHighSpeed>=(respiratorState.periode/3)+deadTime) && timeStampHighSpeed<(2*respiratorState.periode/3))
            {
                etat3=4;                
            }
            break;             
        //MONTEE    
        case 4:
            OC3GeneratePulse(); //On genere un pulse
            if( FIN_COURSE6==0  || (timeStampHighSpeed>=(2*respiratorState.periode/3)-deadTime))
            {
                etat3=5;
            }
            break;
                 
        //ATTENTE    
        case 5:
            if(timeStampHighSpeed>=(2*respiratorState.periode/3))
            {
                etat3=1;
            }
            break;
    }
}
//</editor-fold>


void Timer2CallBack(void)
{
    timeStampHighSpeed++;
}
            


//Timer generation des pulses
void Timer3CallBack(void)
{
    if(respiratorState.flagDoStepsCMD)
    {
        if(respiratorState.doStepsCount>0)
        {
            if(respiratorState.doStepsMotorNum==1)
            {
                DIR1=SENS_MONTEE;
                OC1GeneratePulse(); //On genere un pulse
            }
            else if(respiratorState.doStepsMotorNum==2)
            {
                DIR2=SENS_MONTEE;
                OC2GeneratePulse(); //On genere un pulse
            }
            else
            {
                DIR3=SENS_MONTEE;
                OC3GeneratePulse(); //On genere un pulse
            }
            respiratorState.doStepsCount--;
        }
        else if(respiratorState.doStepsCount<0)
        {
            if(respiratorState.doStepsMotorNum==1)
            {
                DIR1=SENS_DESCENTE;
                OC1GeneratePulse(); //On genere un pulse
            }
            else if(respiratorState.doStepsMotorNum==2)
            {
                DIR2=SENS_DESCENTE;
                OC2GeneratePulse(); //On genere un pulse
            }
            else
            {
                DIR3=SENS_DESCENTE;
                OC3GeneratePulse(); //On genere un pulse
            }
            respiratorState.doStepsCount++;
        }
        else
        {
            respiratorState.flagDoStepsCMD=0;
        }
    }
    else
    {
        if(boardVersion3Moteur)
        {
            StateMachineMoteur1();
            StateMachineMoteur2();
            StateMachineMoteur3();
        }
        else
        {
            if(respiratorState.sensMoteur1==SENS_MONTEE)
            {
                //Si on es dans le sens positif
                OC1GeneratePulse(); //On genere un pulse
                respiratorState.cptMoteur1++;              //On incremente le compteur

                if(respiratorState.cptMoteur1>=respiratorState.amplitude)
                {
                    flagFinMontee=1;
                    TurnOnOffTimer(TIMER3_ID,OFF);      //On arrete le timer
                }
            }
            else
            {
                //Si on es dans le sens negatif
                OC1GeneratePulse(); //On genere un pulse
                respiratorState.cptMoteur1--;              //On decremente le compteur
                if(FIN_COURSE1==0)
                {
                    flagFinDescente=1;
                    TurnOnOffTimer(TIMER3_ID,OFF);      //On arrete le timer
                }
                if(respiratorState.cptMoteur1<=-30)
                {
                    flagFinDescente=1;
                    TurnOnOffTimer(TIMER3_ID,OFF);      //On arrete le timer
                }
            }
        }
    }
}

extern unsigned int antiBlockCounterI2C1;
void Timer4CallBack(void)
{
    g_longTimeStamp++;
    antiBlockCounterI2C1++;
    //ADC1StartConversionSequence();
}


void Timer5CallBack(void)
{
    //respiratorState.volumeCourant+=respiratorState.debitCourant/50.0;
    static unsigned char subdiv=0;
    unsigned char payload[16];
    payload[0]=BREAK_UINT32(g_longTimeStamp,3);
    payload[1]=BREAK_UINT32(g_longTimeStamp,2);
    payload[2]=BREAK_UINT32(g_longTimeStamp,1);
    payload[3]=BREAK_UINT32(g_longTimeStamp,0);
    getBytesFromFloat(payload, 4, respiratorState.pressure1);
    getBytesFromFloat(payload, 8, respiratorState.pressure2);
    getBytesFromFloat(payload, 12, (float)respiratorState.cptMoteur1);
    MakeAndSendMessageWithUTLNProtocol(0x0064, 16, payload);
    unsigned char i2cOut[1];
    unsigned char i2cIn[2];
    I2C1WriteNReadNInterrupt( (0x28<<1), i2cOut,0, i2cIn, 2 );
    respiratorState.pressure2= (float)(((BUILD_UINT16((i2cIn[0]&0x3F),i2cIn[1])-1400))*0.257724);
    
    if(respiratorState.pressure2<0)
        respiratorState.pressure2=0;
//    ADC1StartConversionSequence();
    if(subdiv++>=1)
    {
        subdiv=0;
        respiratorState.pressure1=D6FPress_meas();
        
        
        if(boardVersion3Moteur)
        {
            
        }
        else
        {
            char sign=1;
            float diffPression=respiratorState.pressure1;
            if(diffPression<0)
                sign=-1;
            else
                sign=1;
            
            float vitesse = sqrt(2*ABS(diffPression)/1.23)*(float)sign;
            respiratorState.debitCourant=vitesse*surface;
            
            if(vitesse>=0.01)
                respiratorState.volumeCourant+=(respiratorState.debitCourant*1000)/25.0;
        }
    }
}

void CalculateRespiratorParameters(void)
{
    //respiratorState.periode=2000000;    //(en us)
    
    //
    double periode=60000/respiratorState.cyclesPerMinute;       //en ms
    respiratorState.attenteHaut=0.3*periode;
    respiratorState.attenteBas=0.5*periode;
    
}
