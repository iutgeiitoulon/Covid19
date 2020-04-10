#include "IO.h"
#include <p33FJ128MC804.h>

void InitIO() {
    // IMPORTANT : d�sactive les entr�es analogiques, sinon on perd les entr�es num�riques (Pin remapable)
    AD1PCFGL=0xFFFF;

    //*************************************************************************/
    // Configuration des sorties
    //*************************************************************************/

    //LED
    _TRISA7 = 0; //Led 1 sur RA7
    
    _TRISA9=0;      //DIR
    _TRISA10 = 0; //Led 3 sur RA10
    
    _TRISB2=1;      //FIN_COURSE2
    _TRISB3=1;      //FIN_COURSE1
    _TRISB5=0;      //ELECTROVANNE
    _TRISC6=0;      //STEP
    _TRISC7 = 0; //Led Pilot sur RC7
    
    //*************************************************************************/
    // Configuration des entr�es
    //*************************************************************************/

    //*************************************************************************/
    //Configuration des pins remappables
    //*************************************************************************/
    UnlockIO();
    
    RPOR11bits.RP22R=0b10010;           //Output compare 1 on RP22
    LockIO();
}

void LockIO()
{
    asm volatile ("mov #OSCCON,w1 \n"
                  "mov #0x46, w2 \n"
                  "mov #0x57, w3 \n"
                  "mov.b w2,[w1] \n"
                  "mov.b w3,[w1] \n"
                  "bset OSCCON, #6":::"w1","w2","w3");
}

void UnlockIO()
{
    asm volatile ("mov #OSCCON,w1 \n"
                  "mov #0x46, w2 \n"
                  "mov #0x57, w3 \n"
                  "mov.b w2,[w1] \n"
                  "mov.b w3,[w1] \n"
                  "bclr OSCCON, #6":::"w1","w2","w3");
}